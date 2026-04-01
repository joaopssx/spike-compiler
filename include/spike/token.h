#ifndef SPIKE_TOKEN_H
#define SPIKE_TOKEN_H

#include <cstddef>
#include <string>

namespace spike {

enum class TokenType {
    Algoritmo,
    Var,
    Inicio,
    FimAlgoritmo,
    Inteiro,
    Real,
    Texto,
    Leia,
    Escreva,

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

    Colon,
    Comma,
    LeftParen,
    RightParen,

    Identifier,
    Number,
    String,

    EndOfFile,
};

struct Token {
    TokenType type;
    std::string lexeme;
    std::size_t line;
    std::size_t column;
};

std::string ToString(TokenType type);

}  // namespace spike

#endif  // SPIKE_TOKEN_H
