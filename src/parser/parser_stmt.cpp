#include "spike/parser.hpp"

#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

namespace spike {

// ─── Statements (atribuição, escreva/escreval, leia) ───────────────────────

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
    std::vector<SourcePos> positions;
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
            const Token& name_tok = advance();
            names.push_back(name_tok.lexeme);
            positions.push_back(SourcePos{name_tok.line, name_tok.col});
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
    s.var_positions = std::move(positions);
    s.line = op_tok.line;
    s.col = op_tok.col;
    return make_stmt(std::move(s));
}

} // namespace spike
