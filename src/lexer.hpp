#ifndef SPIKE_LEXER_HPP
#define SPIKE_LEXER_HPP

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include "token.hpp"

class LexerError : public std::runtime_error {
public:
    LexerError(std::string message, std::size_t line, std::size_t column);

    std::size_t line() const noexcept;
    std::size_t column() const noexcept;

private:
    std::size_t line_;
    std::size_t column_;
};

class Lexer {
public:
    explicit Lexer(std::string source);

    std::vector<Token> tokenize();

private:
    bool is_at_end() const;
    char peek() const;
    char peek_next() const;
    char advance();
    void skip_whitespace_and_comments();
    Token make_identifier_or_keyword(std::size_t start_index, std::size_t start_line,
                                     std::size_t start_column);
    Token make_number(std::size_t start_index, std::size_t start_line,
                      std::size_t start_column);
    Token make_string(std::size_t start_line, std::size_t start_column);
    [[noreturn]] void throw_error(const std::string& message, std::size_t line,
                                  std::size_t column) const;

    std::string source_;
    std::size_t index_;
    std::size_t line_;
    std::size_t column_;
};

#endif
