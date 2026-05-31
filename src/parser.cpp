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

// ─── Statements (PROMPT 016: atribuição, escreva/escreval, leia) ───────────

StmtList Parser::parse_stmt_list(TokenType stop_at) {
    StmtList list;
    while (!is_at_end() && !check(stop_at)) {
        StmtPtr s = parse_stmt();
        if (s) list.push_back(std::move(s));
        // Se parse_stmt() retornou nullptr, ele já consumiu o token problemático.
    }
    return list;
}

StmtList Parser::parse_stmt_list(std::initializer_list<TokenType> stop_tokens) {
    StmtList list;
    while (!is_at_end()) {
        bool should_stop = false;
        for (TokenType s : stop_tokens) {
            if (check(s)) { should_stop = true; break; }
        }
        if (should_stop) break;
        StmtPtr s = parse_stmt();
        if (s) list.push_back(std::move(s));
    }
    return list;
}

StmtPtr Parser::parse_stmt() {
    // Atribuição: IDENTIFIER seguido de <- ou :=.
    if (check(TokenType::IDENTIFIER) &&
        (peek_next().type == TokenType::ARROW ||
         peek_next().type == TokenType::ASSIGN)) {
        return parse_assign();
    }
    if (match(TokenType::ESCREVA))   return parse_escreva(false);
    if (match(TokenType::ESCREVAL))  return parse_escreva(true);
    if (match(TokenType::LEIA))      return parse_leia();
    if (match(TokenType::SE))        return parse_se();
    if (match(TokenType::ENQUANTO))  return parse_enquanto();
    if (match(TokenType::PARA))      return parse_para();
    if (match(TokenType::REPITA))    return parse_repita();

    // Não reconheceu — emite e consome 1 token pra garantir progresso.
    const Token& t = peek();
    diags_.error("E300",
                 std::string("instrução não reconhecida: '") + t.lexeme + "'",
                 t.line, t.col,
                 "instruções válidas: atribuição (<-), escreva, leia, se, enquanto, para.");
    advance();
    return nullptr;
}

StmtPtr Parser::parse_assign() {
    // Pré-condição (verificada pelo dispatcher): peek é IDENTIFIER, peek_next
    // é ARROW ou ASSIGN. Mesmo assim, validamos pra não depender disso.
    const Token name_tok = advance();  // IDENTIFIER

    if (!(match(TokenType::ARROW) || match(TokenType::ASSIGN))) {
        const Token& t = peek();
        diags_.error("E305", "'<-' ou ':=' esperado na atribuição",
                     t.line, t.col,
                     "use 'x <- 42' ou 'x := 42' pra atribuir.");
        return nullptr;
    }

    ExprPtr value = parse_expr();

    AssignStmt s;
    s.var_name = name_tok.lexeme;
    s.value = std::move(value);
    s.line = name_tok.line;
    s.col = name_tok.col;
    return make_stmt(std::move(s));
}

StmtPtr Parser::parse_escreva(bool newline) {
    // ESCREVA/ESCREVAL já consumido pelo dispatcher (match no parse_stmt).
    const Token op_tok = previous();
    const char* op_name = newline ? "escreval" : "escreva";

    if (!match(TokenType::LPAREN)) {
        const Token& t = peek();
        diags_.error("E301",
                     std::string("'(' esperado após ") + op_name,
                     t.line, t.col,
                     "use parênteses: escreva(\"texto\", variavel).");
        return nullptr;
    }

    std::vector<ExprPtr> args;
    // `escreva()` sem args é permitido (ex: escreval() pra linha em branco).
    if (!check(TokenType::RPAREN)) {
        do {
            args.push_back(parse_expr());
        } while (match(TokenType::COMMA));
    }

    if (!match(TokenType::RPAREN)) {
        const Token& t = peek();
        diags_.error("E302",
                     std::string("')' esperado no final do ") + op_name,
                     t.line, t.col,
                     "feche a chamada com ')'.");
    }

    EscrevaStmt s;
    s.args = std::move(args);
    s.newline = newline;
    s.line = op_tok.line;
    s.col = op_tok.col;
    return make_stmt(std::move(s));
}

StmtPtr Parser::parse_se() {
    const Token se_tok = previous();  // SE consumido pelo dispatcher

    ExprPtr cond = parse_expr();

    if (!match(TokenType::ENTAO)) {
        const Token& t = peek();
        diags_.error("E400", "'entao' esperado após condição do 'se'",
                     t.line, t.col,
                     "sintaxe: se <condicao> entao ... fimse.");
    }

    StmtList then_branch = parse_stmt_list(
        {TokenType::SENAO, TokenType::FIM_SE});

    StmtList else_branch;
    if (match(TokenType::SENAO)) {
        else_branch = parse_stmt_list(TokenType::FIM_SE);
    }

    if (!match(TokenType::FIM_SE)) {
        const Token& t = peek();
        diags_.error("E401",
                     "'fimse' não encontrado — todo 'se' precisa de um 'fimse'",
                     t.line, t.col, "feche o bloco com 'fimse'.");
    }

    SeStmt s;
    s.condition = std::move(cond);
    s.then_branch = std::move(then_branch);
    s.else_branch = std::move(else_branch);
    s.line = se_tok.line;
    s.col = se_tok.col;
    return make_stmt(std::move(s));
}

StmtPtr Parser::parse_enquanto() {
    const Token enq_tok = previous();  // ENQUANTO consumido pelo dispatcher

    ExprPtr cond = parse_expr();

    if (!match(TokenType::FACA)) {
        const Token& t = peek();
        diags_.error("E500", "'faca' esperado após condição do 'enquanto'",
                     t.line, t.col,
                     "sintaxe: enquanto <condicao> faca ... fimenquanto.");
    }

    StmtList body = parse_stmt_list(TokenType::FIM_ENQUANTO);

    if (!match(TokenType::FIM_ENQUANTO)) {
        const Token& t = peek();
        diags_.error("E501", "'fimenquanto' não encontrado",
                     t.line, t.col, "feche o 'enquanto' com 'fimenquanto'.");
    }

    EnquantoStmt s;
    s.condition = std::move(cond);
    s.body = std::move(body);
    s.line = enq_tok.line;
    s.col = enq_tok.col;
    return make_stmt(std::move(s));
}

StmtPtr Parser::parse_para() {
    const Token para_tok = previous();  // PARA consumido pelo dispatcher

    if (!check(TokenType::IDENTIFIER)) {
        const Token& t = peek();
        diags_.error("E502", "nome de variável esperado após 'para'",
                     t.line, t.col,
                     "sintaxe: para <variavel> de <inicio> ate <fim> faca.");
        return nullptr;
    }
    const Token name_tok = advance();

    if (!match(TokenType::DE)) {
        const Token& t = peek();
        diags_.error("E503",
                     "'de' esperado: sintaxe é 'para i de 1 ate 10 faca'",
                     t.line, t.col, "");
    }

    ExprPtr from = parse_expr();

    if (!match(TokenType::ATE)) {
        const Token& t = peek();
        diags_.error("E504", "'ate' esperado após valor inicial do 'para'",
                     t.line, t.col, "");
    }

    ExprPtr to = parse_expr();

    ExprPtr step = nullptr;
    if (match(TokenType::PASSO)) {
        step = parse_expr();
    }

    if (!match(TokenType::FACA)) {
        const Token& t = peek();
        diags_.error("E505", "'faca' esperado após 'ate [valor]'",
                     t.line, t.col, "");
    }

    StmtList body = parse_stmt_list(TokenType::FIM_PARA);

    if (!match(TokenType::FIM_PARA)) {
        const Token& t = peek();
        diags_.error("E506", "'fimpara' não encontrado",
                     t.line, t.col, "feche o 'para' com 'fimpara'.");
    }

    ParaStmt s;
    s.var_name = name_tok.lexeme;
    s.from = std::move(from);
    s.to = std::move(to);
    s.step = std::move(step);
    s.body = std::move(body);
    s.line = para_tok.line;
    s.col = para_tok.col;
    return make_stmt(std::move(s));
}

StmtPtr Parser::parse_repita() {
    const Token rep_tok = previous();  // REPITA consumido pelo dispatcher

    // Em Portugol o lexer mapeia "ate" pro token ATE (mesmo do `para`).
    // ATE_QUE existe no enum mas não é emitido pelo lexer atual; mantemos
    // os dois como stop pra ficar pronto se algum dia o tokenizador
    // diferenciar "ate que" como token composto.
    StmtList body = parse_stmt_list({TokenType::ATE, TokenType::ATE_QUE});

    if (!(match(TokenType::ATE) || match(TokenType::ATE_QUE))) {
        const Token& t = peek();
        diags_.error("E600", "'ate' esperado para fechar 'repita'",
                     t.line, t.col,
                     "sintaxe: repita ... ate <condicao>. "
                     "O corpo executa pelo menos uma vez e para quando a condição for verdadeira.");
        // Continua mesmo assim — parse_expr abaixo cria stub se necessário.
    }

    ExprPtr cond = parse_expr();

    RepitaStmt s;
    s.body = std::move(body);
    s.condition = std::move(cond);
    s.line = rep_tok.line;
    s.col = rep_tok.col;
    return make_stmt(std::move(s));
}

StmtPtr Parser::parse_leia() {
    // LEIA já consumido pelo dispatcher.
    const Token op_tok = previous();

    if (!match(TokenType::LPAREN)) {
        const Token& t = peek();
        diags_.error("E303", "'(' esperado após leia",
                     t.line, t.col,
                     "use parênteses: leia(x, y).");
        return nullptr;
    }

    std::vector<std::string> names;
    if (!check(TokenType::RPAREN)) {
        while (true) {
            if (!check(TokenType::IDENTIFIER)) {
                const Token& t = peek();
                diags_.error("Exxx", "nome de variável esperado em leia",
                             t.line, t.col,
                             "leia só aceita nomes de variáveis: leia(x, y).");
                // Consome pra evitar travar e sai.
                if (!is_at_end()) advance();
                break;
            }
            names.push_back(advance().lexeme);
            if (!match(TokenType::COMMA)) break;
        }
    }

    if (!match(TokenType::RPAREN)) {
        const Token& t = peek();
        diags_.error("E304", "')' esperado no final do leia",
                     t.line, t.col,
                     "feche a chamada com ')'.");
    }

    LeiaStmt s;
    s.var_names = std::move(names);
    s.line = op_tok.line;
    s.col = op_tok.col;
    return make_stmt(std::move(s));
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
