#include "spike/parser.hpp"

#include <string>
#include <utility>

namespace spike {

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

} // namespace spike
