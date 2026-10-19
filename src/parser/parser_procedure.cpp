#include "spike/parser.hpp"
#include "parser/parser_internal.hpp"

#include <string>
#include <utility>
#include <vector>

namespace spike {

using parser_detail::is_type_keyword;
using parser_detail::type_kind_for;

namespace {

constexpr const char* kHintTypes = "tipos válidos: inteiro, real, caractere, logico.";

std::string params_hint(bool function) {
    return function ? "formato: funcao nome(a: inteiro, b: real): tipo; sem parâmetros, use "
                      "nome()."
                    : "formato: procedimento nome(a: inteiro, b: real); sem parâmetros, use "
                      "nome().";
}

std::string owner_text(bool function, const std::string& name) {
    return (function ? "da função '" : "do procedimento '") + name + "'";
}

bool starts_expression(TokenType type) {
    return type == TokenType::NUMBER || type == TokenType::STRING ||
           type == TokenType::IDENTIFIER || type == TokenType::LPAREN ||
           type == TokenType::MINUS || type == TokenType::NAO || type == TokenType::VERDADEIRO ||
           type == TokenType::FALSO;
}

} // namespace

ProcedureDecl Parser::parse_subprogram(bool function) {
    const Token keyword = previous();
    ProcedureDecl proc;
    proc.line = keyword.line;
    proc.col = keyword.col;
    if (function) proc.return_type = TypeKind::Invalid;
    if (!check(TokenType::IDENTIFIER)) {
        const Token& t = peek();
        diags_.error("E830",
                     function ? "nome da função esperado depois de 'funcao'"
                              : "nome do procedimento esperado depois de 'procedimento'",
                     t.line, t.col, params_hint(function));
        skip_line(keyword.line);
    } else {
        const Token name = advance();
        proc.name = name.lexeme;
        proc.line = name.line;
        proc.col = name.col;
        const bool header_ok = parse_params(proc.params, name.lexeme, function) &&
                               (!function || parse_return_type(proc));
        if (!header_ok) skip_line(name.line);
    }

    if (match(TokenType::VAR)) proc.locals = parse_var_section();
    if (!match(TokenType::INICIO)) {
        const Token& t = peek();
        diags_.error("E102", "inicio esperado", t.line, t.col,
                     function ? "depois do cabeçalho e do var da função, abra o corpo com 'inicio'."
                              : "depois do cabeçalho e do var do procedimento, abra o corpo com "
                                "'inicio'.");
    }
    proc.body = parse_stmt_list({TokenType::FIM_PROCEDIMENTO, TokenType::FIM_FUNCAO,
                                 TokenType::PROCEDIMENTO, TokenType::FUNCAO, TokenType::INICIO,
                                 TokenType::FIM_ALGORITMO});
    if (match(function ? TokenType::FIM_FUNCAO : TokenType::FIM_PROCEDIMENTO)) {
        proc.end_line = previous().line;
        return proc;
    }

    const Token& t = peek();
    const std::string name = !proc.name.empty() ? "'" + proc.name + "'"
                             : function         ? std::string("a função")
                                                : std::string("o procedimento");
    if (function) {
        diags_.error("E836", "'fimfuncao' esperado para fechar " + name, t.line, t.col,
                     "toda função termina com 'fimfuncao', antes do 'inicio' do programa.");
    } else {
        diags_.error("E832", "'fimprocedimento' esperado para fechar " + name, t.line, t.col,
                     "todo procedimento termina com 'fimprocedimento', antes do 'inicio' do "
                     "programa.");
    }
    if (check(TokenType::FIM_PROCEDIMENTO) || check(TokenType::FIM_FUNCAO)) advance();
    return proc;
}

bool Parser::parse_return_type(ProcedureDecl& proc) {
    const std::string hint = "formato: funcao " + proc.name + "(a: inteiro): inteiro.";
    if (!match(TokenType::COLON)) {
        const Token& t = peek();
        diags_.error("E835",
                     "':' e o tipo de retorno esperados depois dos parâmetros da função '" +
                         proc.name + "'",
                     t.line, t.col, hint);
        return false;
    }
    if (check(TokenType::VETOR)) {
        const Token& t = peek();
        diags_.error("E835", "uma função ainda não pode devolver um vetor", t.line, t.col,
                     "devolva um valor simples, ou guarde o resultado num vetor global.");
        return false;
    }
    if (!is_type_keyword(peek().type)) {
        const Token& t = peek();
        diags_.error("E835", "tipo de retorno da função '" + proc.name + "' esperado", t.line,
                     t.col, kHintTypes);
        return false;
    }
    proc.return_type = type_kind_for(advance().type);
    return true;
}

bool Parser::parse_params(std::vector<Param>& params, const std::string& owner, bool function) {
    const std::string hint = params_hint(function);
    if (!match(TokenType::LPAREN)) {
        const Token& t = peek();
        diags_.error("E831", "'(' esperado depois do nome " + owner_text(function, owner), t.line,
                     t.col, hint);
        return false;
    }
    if (match(TokenType::RPAREN)) return true;
    while (true) {
        std::vector<Token> names;
        do {
            if (!check(TokenType::IDENTIFIER)) {
                const Token& t = peek();
                diags_.error("E831", "nome do parâmetro esperado", t.line, t.col, hint);
                return false;
            }
            names.push_back(advance());
        } while (match(TokenType::COMMA));
        if (!match(TokenType::COLON)) {
            const Token& t = peek();
            diags_.error("E831", "':' esperado depois do parâmetro '" + names.back().lexeme + "'",
                         t.line, t.col, hint);
            return false;
        }
        if (check(TokenType::VETOR)) {
            const Token& t = peek();
            diags_.error("E831", "um parâmetro ainda não pode ser um vetor", t.line, t.col,
                         "declare o vetor no var do programa; procedimentos e funções enxergam "
                         "as variáveis globais.");
            return false;
        }
        if (!is_type_keyword(peek().type)) {
            const Token& t = peek();
            diags_.error("E831", "tipo do parâmetro '" + names.back().lexeme + "' esperado",
                         t.line, t.col, kHintTypes);
            return false;
        }
        const TypeKind type = type_kind_for(advance().type);
        for (const Token& name : names) {
            params.push_back(Param{name.lexeme, type, name.line, name.col});
        }
        if (match(TokenType::COMMA)) continue;
        if (match(TokenType::RPAREN)) return true;
        const Token& t = peek();
        diags_.error("E831", "')' esperado no fim dos parâmetros " + owner_text(function, owner),
                     t.line, t.col, hint);
        return false;
    }
}

std::vector<ExprPtr> Parser::parse_call_args(const Token& name) {
    std::vector<ExprPtr> args;
    if (!check(TokenType::RPAREN)) {
        do {
            args.push_back(parse_expr());
        } while (match(TokenType::COMMA));
    }
    if (!match(TokenType::RPAREN)) {
        const Token& t = peek();
        diags_.error("E834", "')' esperado no fim da chamada de '" + name.lexeme + "'", t.line,
                     t.col, "feche a chamada com ')': " + name.lexeme + "(a, b).");
    }
    return args;
}

StmtPtr Parser::parse_call() {
    const Token name = advance();
    advance();
    CallStmt call;
    call.name = name.lexeme;
    call.line = name.line;
    call.col = name.col;
    call.args = parse_call_args(name);
    return make_stmt(std::move(call));
}

ExprPtr Parser::parse_call_expr(const Token& name) {
    advance();
    CallExpr call;
    call.name = name.lexeme;
    call.line = name.line;
    call.col = name.col;
    call.args = parse_call_args(name);
    return make_expr(std::move(call));
}

StmtPtr Parser::parse_retorne() {
    const Token keyword = previous();
    ReturnStmt stmt;
    stmt.line = keyword.line;
    stmt.col = keyword.col;
    if (!is_at_end() && peek().line == keyword.line && starts_expression(peek().type)) {
        stmt.value = parse_expr();
    }
    return make_stmt(std::move(stmt));
}

StmtPtr Parser::reject_late_procedure() {
    const Token keyword = previous();
    const bool function = keyword.type == TokenType::FUNCAO;
    diags_.error("E833",
                 function ? "função declarada depois do 'inicio' do programa"
                          : "procedimento declarado depois do 'inicio' do programa",
                 keyword.line, keyword.col,
                 "declare procedimentos e funções entre o var e o 'inicio' do programa.");
    const TokenType end = function ? TokenType::FIM_FUNCAO : TokenType::FIM_PROCEDIMENTO;
    while (!is_at_end() && !check(TokenType::FIM_ALGORITMO) && !match(end)) advance();
    return nullptr;
}

} // namespace spike
