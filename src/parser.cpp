#include "spike/parser.hpp"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace spike {

namespace {

const char* type_name_for(TokenType t) {
    switch (t) {
        case TokenType::INTEIRO:   return "inteiro";
        case TokenType::REAL:      return "real";
        case TokenType::CARACTERE: return "caractere";
        case TokenType::LOGICO:    return "logico";
        default:                   return "";
    }
}

bool is_type_keyword(TokenType t) {
    return t == TokenType::INTEIRO  || t == TokenType::REAL ||
           t == TokenType::CARACTERE || t == TokenType::LOGICO;
}

// Forma canônica (lowercase, símbolo) do operador, independente de como o
// usuário digitou. `E`/`OU`/`NAO` viram `"e"`/`"ou"`/`"nao"`; símbolos são
// passados como o lexema.
const char* canonical_op(TokenType t) {
    switch (t) {
        case TokenType::PLUS:       return "+";
        case TokenType::MINUS:      return "-";
        case TokenType::STAR:       return "*";
        case TokenType::SLASH:      return "/";
        case TokenType::EQUAL:      return "=";
        case TokenType::NOT_EQUAL:  return "<>";
        case TokenType::LESS:       return "<";
        case TokenType::GREATER:    return ">";
        case TokenType::LESS_EQ:    return "<=";
        case TokenType::GREATER_EQ: return ">=";
        case TokenType::E:          return "e";
        case TokenType::OU:         return "ou";
        case TokenType::NAO:        return "nao";
        default:                    return "?";
    }
}

// Hints — co-localizadas pra ficar fácil refinar a tradução.
constexpr const char* kHintE100 =
    "todo programa Portugol começa com 'algoritmo \"nome\"'.";
constexpr const char* kHintE101 =
    "use aspas duplas: algoritmo \"calculadora\".";
constexpr const char* kHintE102 =
    "depois de 'var' e das declarações, abra o corpo com 'inicio'.";
constexpr const char* kHintE103 =
    "todo 'algoritmo' precisa fechar com 'fimalgoritmo'.";
constexpr const char* kHintE104 =
    "tipos válidos: inteiro, real, caractere, logico.";
constexpr const char* kHintE105 =
    "formato: nome1, nome2 : tipo.";

} // namespace

Parser::Parser(std::vector<Token> tokens, const std::string& filename,
               const std::string& source)
    : tokens_(std::move(tokens)),
      current_(0),
      diags_(filename, source) {}

Program Parser::parse() {
    return parse_program();
}

bool Parser::had_error() const {
    return diags_.has_errors();
}

const Diagnostics& Parser::diagnostics() const {
    return diags_;
}

// ─── Navegação ──────────────────────────────────────────────────────────────

Token& Parser::peek() {
    return tokens_[static_cast<std::size_t>(current_)];
}

Token& Parser::peek_next() {
    if (current_ + 1 >= static_cast<int>(tokens_.size())) {
        // EOF token está sempre no fim; clamp.
        return tokens_.back();
    }
    return tokens_[static_cast<std::size_t>(current_ + 1)];
}

Token& Parser::previous() {
    return tokens_[static_cast<std::size_t>(current_ - 1)];
}

bool Parser::is_at_end() const {
    return tokens_[static_cast<std::size_t>(current_)].type ==
           TokenType::END_OF_FILE;
}

bool Parser::check(TokenType t) const {
    return tokens_[static_cast<std::size_t>(current_)].type == t;
}

Token& Parser::advance() {
    if (!is_at_end()) ++current_;
    return previous();
}

bool Parser::match(TokenType t) {
    if (!check(t)) return false;
    advance();
    return true;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType t : types) {
        if (check(t)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::expect(TokenType t, const std::string& error_msg,
                     const std::string& hint) {
    if (check(t)) return advance();
    Token& cur = peek();
    // Fallback genérico — call sites com códigos catalogados usam
    // diags_.error() diretamente.
    diags_.error("Exxx", error_msg, cur.line, cur.col, hint);
    return cur;
}

// ─── Parsing de alto nível ──────────────────────────────────────────────────

Program Parser::parse_program() {
    Program prog;
    prog.line = peek().line;

    // algoritmo
    if (!check(TokenType::ALGORITMO)) {
        const Token& t = peek();
        diags_.error("E100", "algoritmo esperado", t.line, t.col, kHintE100);
        return prog;
    }
    advance();

    // "nome"
    if (check(TokenType::STRING)) {
        prog.name = peek().lexeme;
        advance();
    } else {
        const Token& t = peek();
        diags_.error("E101", "nome do algoritmo esperado após 'algoritmo'",
                     t.line, t.col, kHintE101);
        // Continua mesmo sem nome — não vale a pena abortar.
    }

    // var (opcional)
    if (check(TokenType::VAR)) {
        advance();
        prog.globals = parse_var_section();
    }

    // inicio
    if (!check(TokenType::INICIO)) {
        const Token& t = peek();
        diags_.error("E102", "inicio esperado", t.line, t.col, kHintE102);
        return prog;
    }
    advance();

    // corpo (vazio até PROMPT 015+) — só consome tokens até fimalgoritmo.
    prog.body = parse_stmt_list(TokenType::FIM_ALGORITMO);

    // fimalgoritmo
    if (!check(TokenType::FIM_ALGORITMO)) {
        const Token& t = peek();
        diags_.error("E103", "fimalgoritmo esperado", t.line, t.col, kHintE103);
        return prog;
    }
    advance();

    return prog;
}

std::vector<VarDecl> Parser::parse_var_section() {
    std::vector<VarDecl> result;

    while (!is_at_end() && !check(TokenType::INICIO)) {
        // Cada iteração parseia uma linha: NOME (, NOME)* : TIPO
        if (!check(TokenType::IDENTIFIER)) {
            const Token& t = peek();
            diags_.error("E105", "':' esperado na declaração de variável",
                         t.line, t.col, kHintE105);
            // Sync: consome até trocar de linha ou achar INICIO.
            const int err_line = t.line;
            while (!is_at_end() && peek().line == err_line &&
                   !check(TokenType::INICIO)) {
                advance();
            }
            continue;
        }

        std::vector<Token> names;
        names.push_back(advance());  // primeiro IDENTIFIER
        while (match(TokenType::COMMA)) {
            if (!check(TokenType::IDENTIFIER)) {
                const Token& t = peek();
                diags_.error("E105",
                             "esperava nome de variável após ','",
                             t.line, t.col, kHintE105);
                break;
            }
            names.push_back(advance());
        }

        if (!match(TokenType::COLON)) {
            const Token& t = peek();
            diags_.error("E105", "':' esperado na declaração de variável",
                         t.line, t.col, kHintE105);
            // Sync pra próxima linha.
            const int err_line = t.line;
            while (!is_at_end() && peek().line == err_line &&
                   !check(TokenType::INICIO)) {
                advance();
            }
            continue;
        }

        std::string type_str;
        if (is_type_keyword(peek().type)) {
            type_str = type_name_for(peek().type);
            advance();
        } else {
            const Token& bad = peek();
            diags_.error("E104",
                         std::string("tipo inválido '") + bad.lexeme + "'",
                         bad.line, bad.col, kHintE104);
            // Avança 1 pra evitar loop infinito; type fica "".
            if (!is_at_end()) advance();
        }

        for (const Token& nt : names) {
            VarDecl vd;
            vd.name = nt.lexeme;
            vd.type = type_str;
            vd.initializer = nullptr;
            vd.line = nt.line;
            vd.col = nt.col;
            result.push_back(std::move(vd));
        }
    }

    return result;
}

// ─── Stubs para statements (PROMPTs seguintes) ──────────────────────────────

StmtList Parser::parse_stmt_list(TokenType stop_at) {
    // Por enquanto consome tokens até `stop_at` ou EOF, sem construir AST.
    // Quando o parser de statements estiver pronto, este corpo é substituído.
    StmtList list;
    while (!is_at_end() && !check(stop_at)) {
        advance();
    }
    return list;
}

StmtPtr Parser::parse_stmt() {
    return nullptr;
}

// ─── Expressões (PROMPT 015) ────────────────────────────────────────────────

ExprPtr Parser::parse_expr() {
    return parse_or();
}

ExprPtr Parser::parse_or() {
    ExprPtr left = parse_and();
    while (check(TokenType::OU)) {
        Token op = advance();
        ExprPtr right = parse_and();
        left = make_expr(BinaryExpr{
            canonical_op(op.type), std::move(left), std::move(right),
            op.line, op.col});
    }
    return left;
}

ExprPtr Parser::parse_and() {
    ExprPtr left = parse_not();
    while (check(TokenType::E)) {
        Token op = advance();
        ExprPtr right = parse_not();
        left = make_expr(BinaryExpr{
            canonical_op(op.type), std::move(left), std::move(right),
            op.line, op.col});
    }
    return left;
}

ExprPtr Parser::parse_not() {
    if (check(TokenType::NAO)) {
        Token op = advance();
        ExprPtr operand = parse_not();  // recursivo: "nao nao x" funciona
        return make_expr(UnaryExpr{
            "nao", std::move(operand), op.line, op.col});
    }
    return parse_comparison();
}

ExprPtr Parser::parse_comparison() {
    ExprPtr left = parse_term();
    while (check(TokenType::EQUAL)      || check(TokenType::NOT_EQUAL) ||
           check(TokenType::LESS)       || check(TokenType::GREATER)   ||
           check(TokenType::LESS_EQ)    || check(TokenType::GREATER_EQ)) {
        Token op = advance();
        ExprPtr right = parse_term();
        left = make_expr(BinaryExpr{
            canonical_op(op.type), std::move(left), std::move(right),
            op.line, op.col});
    }
    return left;
}

ExprPtr Parser::parse_term() {
    ExprPtr left = parse_factor();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        Token op = advance();
        ExprPtr right = parse_factor();
        left = make_expr(BinaryExpr{
            canonical_op(op.type), std::move(left), std::move(right),
            op.line, op.col});
    }
    return left;
}

ExprPtr Parser::parse_factor() {
    ExprPtr left = parse_unary();
    while (check(TokenType::STAR) || check(TokenType::SLASH)) {
        Token op = advance();
        ExprPtr right = parse_unary();
        left = make_expr(BinaryExpr{
            canonical_op(op.type), std::move(left), std::move(right),
            op.line, op.col});
    }
    return left;
}

ExprPtr Parser::parse_unary() {
    if (check(TokenType::MINUS)) {
        Token op = advance();
        ExprPtr operand = parse_unary();  // recursivo: "--x" funciona
        return make_expr(UnaryExpr{
            "-", std::move(operand), op.line, op.col});
    }
    return parse_primary();
}

ExprPtr Parser::parse_primary() {
    Token tok = peek();  // cópia — vamos avançar abaixo

    if (check(TokenType::NUMBER)) {
        advance();
        const bool has_dot = tok.lexeme.find('.') != std::string::npos;
        // O lexer já validou: stod é seguro.
        const double value = std::stod(tok.lexeme);
        return make_expr(NumberExpr{value, !has_dot, tok.line, tok.col});
    }
    if (check(TokenType::STRING)) {
        advance();
        return make_expr(StringExpr{tok.lexeme, tok.line, tok.col});
    }
    if (check(TokenType::VERDADEIRO)) {
        advance();
        return make_expr(BoolExpr{true, tok.line, tok.col});
    }
    if (check(TokenType::FALSO)) {
        advance();
        return make_expr(BoolExpr{false, tok.line, tok.col});
    }
    if (check(TokenType::IDENTIFIER)) {
        advance();
        return make_expr(VarExpr{tok.lexeme, tok.line, tok.col});
    }
    if (check(TokenType::LPAREN)) {
        advance();
        ExprPtr inner = parse_expr();
        if (!match(TokenType::RPAREN)) {
            const Token& cur = peek();
            diags_.error("E200", "')' esperado",
                         cur.line, cur.col,
                         "feche o parêntese antes de continuar a expressão.");
        }
        return inner;
    }

    // Token inesperado. Emite E201 e CONSOME 1 token (a menos que seja EOF)
    // pra evitar loop infinito em chamadores que iteram esperando progresso.
    diags_.error("E201", "expressão esperada",
                 tok.line, tok.col,
                 "uma expressão pode ser número, string, variável, ou (expressão).");
    if (!is_at_end()) advance();
    return make_expr(NumberExpr{0.0, true, tok.line, tok.col});
}

} // namespace spike
