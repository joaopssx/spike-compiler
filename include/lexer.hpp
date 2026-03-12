#pragma once

#include <string>
#include <vector>

#include "token.hpp"

namespace portugol {

class Lexer {
public:
    explicit Lexer(std::string source);

    std::vector<Token> tokenize();

private:
    bool isAtEnd() const;
    char peek(std::size_t offset = 0) const;
    char advance();
    bool match(char expected);

    void skipIgnored();
    Token scanToken();
    Token scanIdentifier(const SourceLocation& startLocation, std::size_t startOffset);
    Token scanNumber(const SourceLocation& startLocation, std::size_t startOffset);
    Token scanString(const SourceLocation& startLocation);
    Token makeToken(TokenType type, std::size_t startOffset, const SourceLocation& startLocation) const;

    static bool isIdentifierStart(char ch);
    static bool isIdentifierPart(char ch);
    static std::string toLowerAscii(std::string_view text);

    std::string source_;
    std::size_t current_ {0};
    std::size_t line_ {1};
    std::size_t column_ {1};
};

} // namespace portugol
