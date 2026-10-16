#include "spike/parser.hpp"
#include "parser/parser_internal.hpp"

#include <string>
#include <utility>
#include <vector>

namespace spike {

using parser_detail::is_type_keyword;
using parser_detail::type_kind_for;

namespace {

constexpr const char* kHintParams =
    "formato: procedimento nome(a: inteiro, b: real); sem parâmetros, use nome().";

} // namespace

ProcedureDecl Parser::parse_procedure() {
    const Token keyword = previous();
    ProcedureDecl proc;
    proc.line = keyword.line;
    proc.col = keyword.col;
    if (!check(TokenType::IDENTIFIER)) {
        const Token& t = peek();
        diags_.error("E830", "nome do procedimento esperado depois de 'procedimento'", t.line,
                     t.col, kHintParams);
        skip_line(keyword.line);
    } else {
        const Token name = advance();
        proc.name = name.lexeme;
        proc.line = name.line;
        proc.col = name.col;
        if (!parse_params(proc.params, name.lexeme)) skip_line(name.line);
    }

    if (match(TokenType::VAR)) proc.locals = parse_var_section();
    if (!match(TokenType::INICIO)) {
        const Token& t = peek();
        diags_.error("E102", "inicio esperado", t.line, t.col,
                     "depois do cabeçalho e do var do procedimento, abra o corpo com 'inicio'.");
    }
    proc.body = parse_stmt_list({TokenType::FIM_PROCEDIMENTO, TokenType::PROCEDIMENTO,
                                 TokenType::INICIO, TokenType::FIM_ALGORITMO});
    if (!match(TokenType::FIM_PROCEDIMENTO)) {
        const Token& t = peek();
        const std::string name = proc.name.empty() ? "o procedimento" : "'" + proc.name + "'";
        diags_.error("E832", "'fimprocedimento' esperado para fechar " + name, t.line, t.col,
                     "todo procedimento termina com 'fimprocedimento', antes do 'inicio' do "
                     "programa.");
    }
    return proc;
}

bool Parser::parse_params(std::vector<Param>& params, const std::string& procedure) {
    if (!match(TokenType::LPAREN)) {
        const Token& t = peek();
        diags_.error("E831", "'(' esperado depois do nome do procedimento '" + procedure + "'",
                     t.line, t.col, kHintParams);
        return false;
    }
    if (match(TokenType::RPAREN)) return true;
    while (true) {
        std::vector<Token> names;
        do {
            if (!check(TokenType::IDENTIFIER)) {
                const Token& t = peek();
                diags_.error("E831", "nome do parâmetro esperado", t.line, t.col, kHintParams);
                return false;
            }
            names.push_back(advance());
        } while (match(TokenType::COMMA));
        if (!match(TokenType::COLON)) {
            const Token& t = peek();
            diags_.error("E831", "':' esperado depois do parâmetro '" + names.back().lexeme + "'",
                         t.line, t.col, kHintParams);
            return false;
        }
        if (check(TokenType::VETOR)) {
            const Token& t = peek();
            diags_.error("E831", "um parâmetro ainda não pode ser um vetor", t.line, t.col,
                         "declare o vetor no var do programa; o procedimento enxerga as "
                         "variáveis globais.");
            return false;
        }
        if (!is_type_keyword(peek().type)) {
            const Token& t = peek();
            diags_.error("E831", "tipo do parâmetro '" + names.back().lexeme + "' esperado",
                         t.line, t.col, "tipos válidos: inteiro, real, caractere, logico.");
            return false;
        }
        const TypeKind type = type_kind_for(advance().type);
        for (const Token& name : names) {
            params.push_back(Param{name.lexeme, type, name.line, name.col});
        }
        if (match(TokenType::COMMA)) continue;
        if (match(TokenType::RPAREN)) return true;
        const Token& t = peek();
        diags_.error("E831", "')' esperado no fim dos parâmetros de '" + procedure + "'", t.line,
                     t.col, kHintParams);
        return false;
    }
}

StmtPtr Parser::parse_call() {
    const Token name = advance();
    advance();
    CallStmt call;
    call.name = name.lexeme;
    call.line = name.line;
    call.col = name.col;
    if (!check(TokenType::RPAREN)) {
        do {
            call.args.push_back(parse_expr());
        } while (match(TokenType::COMMA));
    }
    if (!match(TokenType::RPAREN)) {
        const Token& t = peek();
        diags_.error("E834", "')' esperado no fim da chamada de '" + name.lexeme + "'", t.line,
                     t.col, "feche a chamada com ')': " + name.lexeme + "(a, b).");
    }
    return make_stmt(std::move(call));
}

StmtPtr Parser::reject_late_procedure() {
    const Token keyword = previous();
    diags_.error("E833", "procedimento declarado depois do 'inicio' do programa", keyword.line,
                 keyword.col,
                 "declare os procedimentos entre o var e o 'inicio' do programa.");
    while (!is_at_end() && !check(TokenType::FIM_ALGORITMO) &&
           !match(TokenType::FIM_PROCEDIMENTO)) {
        advance();
    }
    return nullptr;
}

} // namespace spike
