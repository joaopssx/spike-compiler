#include "spike/parser.hpp"

#include <algorithm>
#include <cstddef>
#include <string>

namespace spike {

namespace {

constexpr int kMaxNesting = 256;

bool is_closing_keyword(TokenType t) {
    switch (t) {
        case TokenType::FIM_SE:
        case TokenType::SENAO:
        case TokenType::FIM_ENQUANTO:
        case TokenType::FIM_PARA:
        case TokenType::ATE:
        case TokenType::ATE_QUE:
        case TokenType::FIM_ESCOLHA:
        case TokenType::CASO:
        case TokenType::OUTROCASO:
        case TokenType::FIM_PROCEDIMENTO:
        case TokenType::FIM_FUNCAO:
        case TokenType::FIM_ALGORITMO:
            return true;
        default:
            return false;
    }
}

bool is_statement_keyword(TokenType t) {
    switch (t) {
        case TokenType::SE:
        case TokenType::ENQUANTO:
        case TokenType::PARA:
        case TokenType::REPITA:
        case TokenType::ESCREVA:
        case TokenType::ESCREVAL:
        case TokenType::LEIA:
        case TokenType::INTERROMPA:
        case TokenType::RETORNE:
        case TokenType::ESCOLHA:
        case TokenType::PROCEDIMENTO:
        case TokenType::FUNCAO:
            return true;
        default:
            return false;
    }
}

} // namespace

Parser::NestingGuard::NestingGuard(Parser& owner) : parser(owner) {
    if (++parser.nesting_ <= kMaxNesting || parser.abandoned_) return;
    const Token& t = parser.peek();
    parser.report("E202", "blocos ou parênteses aninhados demais", t.line, t.col,
                  "separe a expressão em partes com variáveis auxiliares, ou divida o código "
                  "em procedimentos.");
    parser.abandoned_ = true;
    parser.current_ = static_cast<int>(parser.tokens_.size()) - 1;
}

Parser::NestingGuard::~NestingGuard() {
    --parser.nesting_;
}

void Parser::report(const std::string& code, const std::string& message, int line, int col,
                    const std::string& hint) {
    ++error_events_;
    if (abandoned_) return;
    if (!error_positions_.insert({line, col}).second) return;
    diags_.error(code, message, line, col, hint);
}

bool Parser::is_block_word(TokenType t) const {
    if (is_closing_keyword(t)) return true;
    for (const auto& stops : block_stops_) {
        if (std::find(stops.begin(), stops.end(), t) != stops.end()) return true;
    }
    return false;
}

bool Parser::closes_open_block() const {
    const TokenType t = tokens_[static_cast<std::size_t>(current_)].type;
    for (const auto& stops : block_stops_) {
        if (std::find(stops.begin(), stops.end(), t) != stops.end()) return true;
    }
    return false;
}

bool Parser::starts_statement() {
    if (is_statement_keyword(peek().type)) return true;
    return check(TokenType::IDENTIFIER) && (current_ == 0 || previous().line < peek().line);
}

bool Parser::looks_like_statement() {
    if (check(TokenType::FIM_ALGORITMO) || is_statement_keyword(peek().type)) return true;
    const TokenType next = peek_next().type;
    return check(TokenType::IDENTIFIER) &&
           (next == TokenType::ARROW || next == TokenType::ASSIGN || next == TokenType::LPAREN);
}

void Parser::synchronize(int error_line) {
    while (!is_at_end() && !is_block_word(peek().type)) {
        if (peek().line > error_line && starts_statement()) return;
        advance();
    }
}

bool Parser::has_token_ahead(TokenType t) const {
    for (std::size_t i = static_cast<std::size_t>(current_); i < tokens_.size(); ++i) {
        if (tokens_[i].type == t) return true;
    }
    return false;
}

SourcePos error_after(const Token& token) {
    return SourcePos{token.line, token.col + token.length};
}

SourcePos Parser::expected_pos() {
    if (current_ > 0 && (is_at_end() || peek().line > previous().line)) {
        return error_after(previous());
    }
    return SourcePos{peek().line, peek().col};
}

ExprPtr Parser::missing_expr() {
    const SourcePos pos = expected_pos();
    report("E201", "expressão esperada", pos.line, pos.col,
           "uma expressão pode ser número, string, variável, ou (expressão).");
    return make_expr(ErrorExpr{pos.line, pos.col});
}

} // namespace spike
