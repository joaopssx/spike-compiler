#include "spike/parser.hpp"

#include <string>
#include <utility>

namespace spike {

StmtPtr Parser::parse_se() {
    const Token se_tok = previous();  // SE consumido pelo dispatcher

    ExprPtr cond = parse_expr();

    if (!match(TokenType::ENTAO)) {
        const SourcePos t = expected_pos();
        report("E400", "'entao' esperado após condição do 'se'",
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
        report("E401",
               "'fimse' não encontrado — todo 'se' precisa de um 'fimse'",
               se_tok.line, se_tok.col, "feche o bloco com 'fimse'.");
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
        const SourcePos t = expected_pos();
        report("E500", "'faca' esperado após condição do 'enquanto'",
               t.line, t.col,
               "sintaxe: enquanto <condicao> faca ... fimenquanto.");
    }

    StmtList body = parse_stmt_list(TokenType::FIM_ENQUANTO);

    if (!match(TokenType::FIM_ENQUANTO)) {
        report("E501", "'fimenquanto' não encontrado",
               enq_tok.line, enq_tok.col, "feche o 'enquanto' com 'fimenquanto'.");
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

    Token name_tok = peek();
    name_tok.lexeme.clear();
    if (check(TokenType::IDENTIFIER)) {
        name_tok = advance();
    } else {
        report("E502", "nome de variável esperado após 'para'",
               name_tok.line, name_tok.col,
               "sintaxe: para <variavel> de <inicio> ate <fim> faca.");
    }

    if (!match(TokenType::DE)) {
        const SourcePos t = expected_pos();
        report("E503",
               "'de' esperado: sintaxe é 'para i de 1 ate 10 faca'",
               t.line, t.col, "");
    }

    ExprPtr from = parse_expr();

    if (!match(TokenType::ATE)) {
        const SourcePos t = expected_pos();
        report("E504", "'ate' esperado após valor inicial do 'para'",
               t.line, t.col, "");
    }

    ExprPtr to = parse_expr();

    ExprPtr step = nullptr;
    if (match(TokenType::PASSO)) {
        step = parse_expr();
    }

    if (!match(TokenType::FACA)) {
        const SourcePos t = expected_pos();
        report("E505", "'faca' esperado após 'ate [valor]'",
               t.line, t.col, "");
    }

    StmtList body = parse_stmt_list(TokenType::FIM_PARA);

    if (!match(TokenType::FIM_PARA)) {
        report("E506", "'fimpara' não encontrado",
               para_tok.line, para_tok.col, "feche o 'para' com 'fimpara'.");
    }

    ParaStmt s;
    s.var_name = name_tok.lexeme;
    s.var_pos = SourcePos{name_tok.line, name_tok.col};
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
    // ATE_QUE existe no enum mas nao e emitido pelo lexer atual; mantemos
    // os dois como stop pra ficar pronto se algum dia o tokenizador
    // diferenciar "ate que" como token composto.
    StmtList body = parse_stmt_list({TokenType::ATE, TokenType::ATE_QUE});

    ExprPtr cond;
    if (match(TokenType::ATE) || match(TokenType::ATE_QUE)) {
        cond = parse_expr();
    } else {
        report("E600", "'ate' esperado para fechar 'repita'",
               rep_tok.line, rep_tok.col,
               "sintaxe: repita ... ate <condicao>. "
               "O corpo executa pelo menos uma vez e para quando a condição for verdadeira.");
        cond = make_expr(ErrorExpr{rep_tok.line, rep_tok.col});
    }

    RepitaStmt s;
    s.body = std::move(body);
    s.condition = std::move(cond);
    s.line = rep_tok.line;
    s.col = rep_tok.col;
    return make_stmt(std::move(s));
}

} // namespace spike
