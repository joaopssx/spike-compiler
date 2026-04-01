#ifndef SPIKE_LEXER_H
#define SPIKE_LEXER_H

#include <cstddef>
#include <string>
#include <vector>

#include "spike/token.h"

namespace spike {

class Lexer {
public:
    explicit Lexer(std::string source);

    std::vector<Token> Tokenize();

private:
    bool IsAtEnd() const;
    char Peek(std::size_t offset = 0) const;
    char Advance();
    bool Match(char expected);

    void SkipWhitespace();

    Token ReadIdentifierOrKeyword();
    Token ReadNumber();
    Token ReadString();
    Token BuildToken(TokenType type, std::size_t start, std::size_t line,
                     std::size_t column) const;

    [[noreturn]] void ThrowError(const std::string& message, std::size_t line,
                                 std::size_t column) const;

    std::string source_;
    std::size_t position_;
    std::size_t line_;
    std::size_t column_;
};

}  // namespace spike

#endif  // SPIKE_LEXER_H
