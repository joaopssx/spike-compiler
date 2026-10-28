#include "spike/parser.hpp"
#include "parser/parser_internal.hpp"

#include <string>
#include <utility>
#include <vector>

namespace spike {

using parser_detail::is_type_keyword;
using parser_detail::type_kind_for;

namespace {

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
constexpr const char* kHintConst =
    "formato: NOME = valor, uma constante por linha.";
constexpr const char* kHintE105 =
    "formato: nome1, nome2 : tipo.";

} // namespace

// --- Parsing de alto nivel --------------------------------------------------

Program Parser::parse_program() {
    Program prog;
    prog.line = peek().line;

    // algoritmo
    if (!check(TokenType::ALGORITMO)) {
        const SourcePos t = expected_pos();
        report("E100", "algoritmo esperado", t.line, t.col, kHintE100);
        return prog;
    }
    advance();

    // "nome"
    if (check(TokenType::STRING)) {
        prog.name = peek().lexeme;
        advance();
    } else {
        const SourcePos t = expected_pos();
        report("E101", "nome do algoritmo esperado após 'algoritmo'",
               t.line, t.col, kHintE101);
        // Continua mesmo sem nome - nao vale a pena abortar.
    }

    if (check(TokenType::CONST_KEYWORD)) {
        advance();
        prog.consts = parse_const_section();
    }

    // var (opcional)
    if (check(TokenType::VAR)) {
        advance();
        prog.globals = parse_var_section();
    }

    while (check(TokenType::PROCEDIMENTO) || check(TokenType::FUNCAO)) {
        const bool function = advance().type == TokenType::FUNCAO;
        prog.procedures.push_back(parse_subprogram(function));
    }

    // inicio: se faltar, pula ate um 'inicio' mais adiante ou continua como se
    // ele estivesse aqui.
    if (!match(TokenType::INICIO)) {
        const SourcePos t = expected_pos();
        report("E102", "inicio esperado", t.line, t.col, kHintE102);
        if (has_token_ahead(TokenType::INICIO)) {
            while (!match(TokenType::INICIO)) advance();
        }
    }

    // corpo: comandos ate fimalgoritmo.
    prog.body = parse_stmt_list(TokenType::FIM_ALGORITMO);

    // fimalgoritmo: cobrado uma vez, so aqui no fim.
    if (!match(TokenType::FIM_ALGORITMO)) {
        const SourcePos t = expected_pos();
        report("E103", "fimalgoritmo esperado", t.line, t.col, kHintE103);
    }

    return prog;
}

bool Parser::ends_const_section() {
    return check(TokenType::VAR) || check(TokenType::INICIO) || check(TokenType::PROCEDIMENTO) ||
           check(TokenType::FUNCAO);
}

void Parser::skip_line(int line) {
    while (!is_at_end() && peek().line == line && !check(TokenType::VAR) &&
           !check(TokenType::INICIO)) {
        advance();
    }
}

std::vector<ConstDecl> Parser::parse_const_section() {
    std::vector<ConstDecl> result;
    const Token& keyword = previous();
    if (ends_const_section() || is_at_end()) {
        const SourcePos pos = error_after(keyword);
        report("E800", "nome da constante esperado depois de 'const'", pos.line, pos.col,
               kHintConst);
        return result;
    }
    while (!is_at_end() && !ends_const_section()) {
        if (!check(TokenType::IDENTIFIER)) {
            const SourcePos t = expected_pos();
            report("E800", "nome da constante esperado", t.line, t.col, kHintConst);
            skip_line(peek().line);
            continue;
        }
        const Token name = advance();
        if (!match(TokenType::EQUAL)) {
            const SourcePos t = expected_pos();
            report("E801", "'=' esperado depois do nome da constante '" + name.lexeme + "'",
                   t.line, t.col, kHintConst);
            skip_line(name.line);
            continue;
        }
        const Token equal = previous();
        if (is_at_end() || ends_const_section() ||
            peek().line != equal.line) {
            const SourcePos pos = error_after(equal);
            report("E802", "valor esperado para a constante '" + name.lexeme + "'",
                   pos.line, pos.col, kHintConst);
            continue;
        }
        ConstDecl decl;
        decl.name = name.lexeme;
        decl.value = parse_expr();
        decl.line = name.line;
        decl.col = name.col;
        result.push_back(std::move(decl));
    }
    return result;
}

std::vector<VarDecl> Parser::parse_var_section() {
    std::vector<VarDecl> result;

    while (!is_at_end() && !check(TokenType::INICIO) && !check(TokenType::PROCEDIMENTO) &&
           !check(TokenType::FUNCAO) && !looks_like_statement()) {
        // Cada iteracao parseia uma linha: NOME (, NOME)* : TIPO
        if (!check(TokenType::IDENTIFIER)) {
            const SourcePos t = expected_pos();
            report("E105", "':' esperado na declaração de variável",
                   t.line, t.col, kHintE105);
            // Sync: consome ate trocar de linha ou achar INICIO.
            const int err_line = peek().line;
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
                const SourcePos t = expected_pos();
                report("E105",
                       "esperava nome de variável após ','",
                       t.line, t.col, kHintE105);
                break;
            }
            names.push_back(advance());
        }

        if (!match(TokenType::COLON)) {
            const SourcePos t = expected_pos();
            report("E105", "':' esperado na declaração de variável",
                   t.line, t.col, kHintE105);
            // Sync pra proxima linha.
            const int err_line = peek().line;
            while (!is_at_end() && peek().line == err_line &&
                   !check(TokenType::INICIO)) {
                advance();
            }
            continue;
        }

        std::vector<ArrayDim> dims;
        if (match(TokenType::VETOR) && !parse_array_type(dims)) {
            skip_line(names.front().line);
            continue;
        }

        TypeKind type = TypeKind::Invalid;
        if (is_type_keyword(peek().type)) {
            type = type_kind_for(peek().type);
            advance();
        } else {
            const Token& bad = peek();
            report("E104",
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
            vd.dims = dims;
            result.push_back(std::move(vd));
        }
    }

    return result;
}

} // namespace spike
