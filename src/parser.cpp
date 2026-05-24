#include "spike/parser.hpp"

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

// ─── Stubs para statements/expressões (PROMPTs seguintes) ───────────────────

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

ExprPtr Parser::parse_expr() {
    return nullptr;
}

} // namespace spike
