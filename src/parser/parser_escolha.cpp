#include "spike/parser.hpp"

#include <string>
#include <utility>

namespace spike {

namespace {

constexpr const char* kHintEscolha =
    "formato: escolha valor / caso 1 / caso 2, 3 / caso 4 ate 6 / outrocaso / fimescolha.";

} // namespace

bool Parser::parse_case_labels(CaseClause& clause) {
    const Token& keyword = previous();
    if (is_at_end() || peek().line != keyword.line || check(TokenType::CASO) ||
        check(TokenType::OUTROCASO) || check(TokenType::FIM_ESCOLHA)) {
        diags_.error("E813", "valor esperado depois de 'caso'", keyword.line, keyword.col + 4,
                     "escreva um valor (caso 1), uma lista (caso 2, 3) ou um intervalo "
                     "(caso 4 ate 6).");
        return false;
    }
    do {
        CaseLabel label;
        label.line = peek().line;
        label.col = peek().col;
        label.value = parse_expr();
        if (match(TokenType::ATE)) label.range_end = parse_expr();
        clause.labels.push_back(std::move(label));
    } while (match(TokenType::COMMA));
    return true;
}

StmtPtr Parser::parse_escolha() {
    const Token keyword = previous();
    EscolhaStmt s;
    s.line = keyword.line;
    s.col = keyword.col;
    s.selector = parse_expr();

    const auto stops = {TokenType::CASO, TokenType::OUTROCASO, TokenType::FIM_ESCOLHA,
                        TokenType::FIM_ALGORITMO};
    if (!check(TokenType::CASO) && !check(TokenType::OUTROCASO) &&
        !check(TokenType::FIM_ESCOLHA)) {
        const Token& t = peek();
        diags_.error("E810", "'caso' esperado depois de 'escolha'", t.line, t.col, kHintEscolha);
        parse_stmt_list(stops);
    }

    bool closed = false;
    while (!is_at_end() && !check(TokenType::FIM_ALGORITMO)) {
        if (match(TokenType::FIM_ESCOLHA)) {
            closed = true;
            break;
        }
        if (match(TokenType::CASO)) {
            const Token caso = previous();
            if (s.has_default) {
                diags_.error("E812", "'caso' depois de 'outrocaso'", caso.line, caso.col,
                             "'outrocaso' precisa ser o último bloco do 'escolha'.");
            }
            CaseClause clause;
            clause.line = caso.line;
            clause.col = caso.col;
            parse_case_labels(clause);
            clause.body = parse_stmt_list(stops);
            s.cases.push_back(std::move(clause));
            continue;
        }
        if (match(TokenType::OUTROCASO)) {
            const Token outro = previous();
            StmtList body = parse_stmt_list(stops);
            if (s.has_default) {
                diags_.error("E812", "'outrocaso' repetido", outro.line, outro.col,
                             "um 'escolha' tem no máximo um 'outrocaso', no fim.");
                continue;
            }
            s.has_default = true;
            s.default_body = std::move(body);
            continue;
        }
        break;
    }
    if (!closed) {
        const Token& t = peek();
        diags_.error("E811", "'fimescolha' não encontrado", t.line, t.col,
                     "feche o 'escolha' com 'fimescolha'.");
    }
    if (s.cases.empty() && !s.has_default) {
        diags_.error("E814", "'escolha' sem nenhum caso", keyword.line, keyword.col,
                     kHintEscolha);
    }
    return make_stmt(std::move(s));
}

} // namespace spike
