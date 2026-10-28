#include "spike/parser.hpp"

#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

namespace spike {

// --- Statements (atribuicao, escreva/escreval, leia) -----------------------

StmtList Parser::parse_stmt_list(TokenType stop_at) {
    return parse_stmt_list({stop_at});
}

// Para no fechamento deste bloco ou de qualquer bloco aberto em volta: assim um
// 'fimse' esquecido nao engole o 'fimenquanto' de fora.
StmtList Parser::parse_stmt_list(std::initializer_list<TokenType> stop_tokens) {
    block_stops_.emplace_back(stop_tokens);
    StmtList list;
    while (!is_at_end() && !closes_open_block()) {
        const int start = current_;
        const int errors_before = error_events_;
        StmtPtr s = parse_stmt();
        if (s) list.push_back(std::move(s));
        if (error_events_ != errors_before) synchronize(previous().line);
        if (current_ == start) advance();
    }
    block_stops_.pop_back();
    return list;
}

StmtPtr Parser::parse_stmt() {
    const NestingGuard guard(*this);
    if (abandoned_) return nullptr;
    // Atribuicao: IDENTIFIER seguido de <- ou :=.
    if (check(TokenType::IDENTIFIER) &&
        (peek_next().type == TokenType::ARROW || peek_next().type == TokenType::ASSIGN ||
         peek_next().type == TokenType::LBRACKET)) {
        return parse_assign();
    }
    if (check(TokenType::IDENTIFIER) && peek_next().type == TokenType::LPAREN) {
        return parse_call();
    }
    if (match(TokenType::PROCEDIMENTO) || match(TokenType::FUNCAO)) return reject_late_procedure();
    if (match(TokenType::RETORNE)) return parse_retorne();
    if (match(TokenType::ESCREVA))   return parse_escreva(false);
    if (match(TokenType::ESCREVAL))  return parse_escreva(true);
    if (match(TokenType::LEIA))      return parse_leia();
    if (match(TokenType::SE))        return parse_se();
    if (match(TokenType::ENQUANTO))  return parse_enquanto();
    if (match(TokenType::PARA))      return parse_para();
    if (match(TokenType::REPITA))    return parse_repita();
    if (match(TokenType::ESCOLHA))   return parse_escolha();
    if (match(TokenType::INTERROMPA)) {
        const Token& keyword = previous();
        return make_stmt(BreakStmt{keyword.line, keyword.col});
    }

    // Nao reconheceu - emite, consome 1 token e descarta o resto da linha.
    const Token t = peek();
    report("E300",
           std::string("instrução não reconhecida: '") + t.lexeme + "'",
           t.line, t.col,
           "instruções válidas: atribuição (<-), escreva, leia, se, enquanto, para.");
    advance();
    synchronize(t.line);
    return nullptr;
}

StmtPtr Parser::parse_assign() {
    // Pre-condicao (verificada pelo dispatcher): peek e IDENTIFIER, peek_next
    // e ARROW ou ASSIGN. Mesmo assim, validamos pra nao depender disso.
    const Token name_tok = advance();  // IDENTIFIER
    ExprPtr target = parse_name_reference(name_tok);

    if (!(match(TokenType::ARROW) || match(TokenType::ASSIGN))) {
        const SourcePos t = expected_pos();
        report("E305", "'<-' ou ':=' esperado na atribuição",
               t.line, t.col,
               "use 'x <- 42' ou 'x := 42' pra atribuir.");
        return nullptr;
    }

    const bool value_on_next_line = is_at_end() || peek().line > previous().line;
    ExprPtr value = value_on_next_line ? missing_expr() : parse_expr();

    AssignStmt s;
    s.target = std::move(target);
    s.value = std::move(value);
    s.line = name_tok.line;
    s.col = name_tok.col;
    return make_stmt(std::move(s));
}

StmtPtr Parser::parse_escreva(bool newline) {
    // ESCREVA/ESCREVAL ja consumido pelo dispatcher (match no parse_stmt).
    const Token op_tok = previous();
    const char* op_name = newline ? "escreval" : "escreva";

    if (!match(TokenType::LPAREN)) {
        const SourcePos t = expected_pos();
        report("E301",
               std::string("'(' esperado após ") + op_name,
               t.line, t.col,
               "use parênteses: escreva(\"texto\", variavel).");
        return nullptr;
    }

    std::vector<ExprPtr> args;
    // `escreva()` sem args e permitido (ex: escreval() pra linha em branco).
    if (!check(TokenType::RPAREN)) {
        do {
            args.push_back(parse_expr());
        } while (match(TokenType::COMMA));
    }

    if (!match(TokenType::RPAREN)) {
        const SourcePos t = expected_pos();
        report("E302",
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
    // LEIA ja consumido pelo dispatcher.
    const Token op_tok = previous();

    if (!match(TokenType::LPAREN)) {
        const SourcePos t = expected_pos();
        report("E303", "'(' esperado após leia",
               t.line, t.col,
               "use parênteses: leia(x, y).");
        return nullptr;
    }

    std::vector<ExprPtr> targets;
    if (!check(TokenType::RPAREN)) {
        while (true) {
            if (!check(TokenType::IDENTIFIER)) {
                const SourcePos t = expected_pos();
                report("E306", "nome de variável esperado em leia",
                             t.line, t.col,
                             "leia só aceita nomes de variáveis: leia(x, y).");
                // Consome pra evitar travar e sai.
                if (!is_at_end()) advance();
                break;
            }
            const Token& name_tok = advance();
            targets.push_back(parse_name_reference(name_tok));
            if (!match(TokenType::COMMA)) break;
        }
    }

    if (!match(TokenType::RPAREN)) {
        const SourcePos t = expected_pos();
        report("E304", "')' esperado no final do leia",
               t.line, t.col,
               "feche a chamada com ')'.");
    }

    LeiaStmt s;
    s.targets = std::move(targets);
    s.line = op_tok.line;
    s.col = op_tok.col;
    return make_stmt(std::move(s));
}

} // namespace spike
