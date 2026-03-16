#ifndef SPIKE_TOKEN_TYPE_HPP
#define SPIKE_TOKEN_TYPE_HPP

#include <string_view>

enum class TokenType {
    KeywordAlgoritmo,
    KeywordVar,
    KeywordInicio,
    KeywordFimAlgoritmo,
    KeywordInteiro,
    KeywordReal,
    KeywordTexto,
    KeywordLeia,
    KeywordEscreva,
    Identifier,
    IntegerLiteral,
    RealLiteral,
    StringLiteral,
    Plus,
    Minus,
    Star,
    Slash,
    Assign,
    Equal,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    LeftParen,
    RightParen,
    Comma,
    Colon,
    EndOfFile,
};

std::string_view token_type_to_string(TokenType type);

#endif
