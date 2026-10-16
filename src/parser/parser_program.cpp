#include "spike/parser.hpp"

#include <string>
#include <utility>
#include <vector>

namespace spike {

namespace {

TypeKind type_kind_for(TokenType t) {
    switch (t) {
        case TokenType::INTEIRO:   return TypeKind::Integer;
        case TokenType::REAL:      return TypeKind::Real;
        case TokenType::CARACTERE: return TypeKind::String;
        case TokenType::LOGICO:    return TypeKind::Bool;
        default:                   return TypeKind::Invalid;
    }
}

bool is_type_keyword(TokenType t) {
    return t == TokenType::INTEIRO  || t == TokenType::REAL ||
           t == TokenType::CARACTERE || t == TokenType::LOGICO;
}

// Hints - co-localizadas pra ficar facil refinar a traducao.
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

// --- Parsing de alto nivel --------------------------------------------------

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
        // Continua mesmo sem nome - nao vale a pena abortar.
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

    // corpo: comandos ate fimalgoritmo.
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
        // Cada iteracao parseia uma linha: NOME (, NOME)* : TIPO
        if (!check(TokenType::IDENTIFIER)) {
            const Token& t = peek();
            diags_.error("E105", "':' esperado na declaração de variável",
                         t.line, t.col, kHintE105);
            // Sync: consome ate trocar de linha ou achar INICIO.
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
            // Sync pra proxima linha.
            const int err_line = t.line;
            while (!is_at_end() && peek().line == err_line &&
                   !check(TokenType::INICIO)) {
                advance();
            }
            continue;
        }

        TypeKind type = TypeKind::Invalid;
        if (is_type_keyword(peek().type)) {
            type = type_kind_for(peek().type);
            advance();
        } else {
            const Token& bad = peek();
            diags_.error("E104",
                         std::string("tipo inválido '") + bad.lexeme + "'",
                         bad.line, bad.col, kHintE104);
            // Avanca 1 pra evitar loop infinito; type fica Invalid.
            if (!is_at_end()) advance();
        }

        for (const Token& nt : names) {
            VarDecl vd;
            vd.name = nt.lexeme;
            vd.type = type;
            vd.initializer = nullptr;
            vd.line = nt.line;
            vd.col = nt.col;
            result.push_back(std::move(vd));
        }
    }

    return result;
}

} // namespace spike
