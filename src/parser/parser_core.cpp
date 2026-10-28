#include "spike/parser.hpp"

#include <cstddef>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

namespace spike {

Parser::Parser(std::vector<Token> tokens, const std::string& filename,
               const std::string& source)
    : tokens_(std::move(tokens)),
      current_(0),
      diags_(filename, source) {}

Program Parser::parse() {
    return parse_program();
}

bool Parser::had_error() const {
    return diags_.has_errors();
}

const Diagnostics& Parser::diagnostics() const {
    return diags_;
}

// --- Navegacao --------------------------------------------------------------

Token& Parser::peek() {
    return tokens_[static_cast<std::size_t>(current_)];
}

Token& Parser::peek_next() {
    if (current_ + 1 >= static_cast<int>(tokens_.size())) {
        // EOF token esta sempre no fim; clamp.
        return tokens_.back();
    }
    return tokens_[static_cast<std::size_t>(current_ + 1)];
}

Token& Parser::previous() {
    return tokens_[static_cast<std::size_t>(current_ - 1)];
}

bool Parser::is_at_end() const {
    return tokens_[static_cast<std::size_t>(current_)].type ==
           TokenType::END_OF_FILE;
}

bool Parser::check(TokenType t) const {
    return tokens_[static_cast<std::size_t>(current_)].type == t;
}

Token& Parser::advance() {
    if (!is_at_end()) ++current_;
    return previous();
}

bool Parser::match(TokenType t) {
    if (!check(t)) return false;
    advance();
    return true;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType t : types) {
        if (check(t)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::expect(TokenType t, const std::string& error_msg,
                     const std::string& hint) {
    if (check(t)) return advance();
    Token& cur = peek();
    // Fallback generico - call sites com codigos catalogados usam
    // report() diretamente.
    const SourcePos pos = expected_pos();
    report("Exxx", error_msg, pos.line, pos.col, hint);
    return cur;
}

} // namespace spike
