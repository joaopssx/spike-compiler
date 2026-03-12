#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace portugol {

struct SourceLocation {
    std::size_t line {1};
    std::size_t column {1};
    std::size_t offset {0};
};

enum class TokenType {
    EndOfFile,
    Identifier,
    NumberInt,
    NumberReal,
    String,
    Algoritmo,
    Var,
    Inicio,
    FimAlgoritmo,
    Inteiro,
    Real,
    Logico,
    Caractere,
    Se,
    Senao,
    Entao,
    FimSe,
    Enquanto,
    Faca,
    FimEnquanto,
    Escreva,
    Leia,
    LParen,
    RParen,
    Comma,
    Colon,
    Plus,
    Minus,
    Star,
    Slash,
    Assign,
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
};

struct Token {
    TokenType type;
    std::string lexeme;
    SourceLocation location;
};

inline std::string_view tokenTypeName(TokenType type) {
    switch (type) {
    case TokenType::EndOfFile:
        return "EOF";
    case TokenType::Identifier:
        return "Identifier";
    case TokenType::NumberInt:
        return "NumberInt";
    case TokenType::NumberReal:
        return "NumberReal";
    case TokenType::String:
        return "String";
    case TokenType::Algoritmo:
        return "Algoritmo";
    case TokenType::Var:
        return "Var";
    case TokenType::Inicio:
        return "Inicio";
    case TokenType::FimAlgoritmo:
        return "FimAlgoritmo";
    case TokenType::Inteiro:
        return "Inteiro";
    case TokenType::Real:
        return "Real";
    case TokenType::Logico:
        return "Logico";
    case TokenType::Caractere:
        return "Caractere";
    case TokenType::Se:
        return "Se";
    case TokenType::Senao:
        return "Senao";
    case TokenType::Entao:
        return "Entao";
    case TokenType::FimSe:
        return "FimSe";
    case TokenType::Enquanto:
        return "Enquanto";
    case TokenType::Faca:
        return "Faca";
    case TokenType::FimEnquanto:
        return "FimEnquanto";
    case TokenType::Escreva:
        return "Escreva";
    case TokenType::Leia:
        return "Leia";
    case TokenType::LParen:
        return "LParen";
    case TokenType::RParen:
        return "RParen";
    case TokenType::Comma:
        return "Comma";
    case TokenType::Colon:
        return "Colon";
    case TokenType::Plus:
        return "Plus";
    case TokenType::Minus:
        return "Minus";
    case TokenType::Star:
        return "Star";
    case TokenType::Slash:
        return "Slash";
    case TokenType::Assign:
        return "Assign";
    case TokenType::Equal:
        return "Equal";
    case TokenType::NotEqual:
        return "NotEqual";
    case TokenType::Less:
        return "Less";
    case TokenType::LessEqual:
        return "LessEqual";
    case TokenType::Greater:
        return "Greater";
    case TokenType::GreaterEqual:
        return "GreaterEqual";
    }

    return "Unknown";
}

inline std::string tokenToString(const Token& token) {
    std::string text(tokenTypeName(token.type));
    if (!token.lexeme.empty()) {
        text += " -> ";
        text += token.lexeme;
    }
    return text;
}

} // namespace portugol
