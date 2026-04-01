#include "spike/token.h"

#include <stdexcept>

namespace spike {

std::string ToString(TokenType type) {
    switch (type) {
        case TokenType::Algoritmo:
            return "TOKEN_ALGORITMO";
        case TokenType::Var:
            return "TOKEN_VAR";
        case TokenType::Inicio:
            return "TOKEN_INICIO";
        case TokenType::FimAlgoritmo:
            return "TOKEN_FIMALGORITMO";
        case TokenType::Inteiro:
            return "TOKEN_INTEIRO";
        case TokenType::Real:
            return "TOKEN_REAL";
        case TokenType::Texto:
            return "TOKEN_TEXTO";
        case TokenType::Leia:
            return "TOKEN_LEIA";
        case TokenType::Escreva:
            return "TOKEN_ESCREVA";
        case TokenType::Plus:
            return "TOKEN_PLUS";
        case TokenType::Minus:
            return "TOKEN_MINUS";
        case TokenType::Star:
            return "TOKEN_STAR";
        case TokenType::Slash:
            return "TOKEN_SLASH";
        case TokenType::Assign:
            return "TOKEN_ASSIGN";
        case TokenType::Equal:
            return "TOKEN_EQUAL";
        case TokenType::Less:
            return "TOKEN_LESS";
        case TokenType::Greater:
            return "TOKEN_GREATER";
        case TokenType::LessEqual:
            return "TOKEN_LESS_EQUAL";
        case TokenType::GreaterEqual:
            return "TOKEN_GREATER_EQUAL";
        case TokenType::Colon:
            return "TOKEN_COLON";
        case TokenType::Comma:
            return "TOKEN_COMMA";
        case TokenType::LeftParen:
            return "TOKEN_LEFT_PAREN";
        case TokenType::RightParen:
            return "TOKEN_RIGHT_PAREN";
        case TokenType::Identifier:
            return "TOKEN_IDENTIFIER";
        case TokenType::Number:
            return "TOKEN_NUMBER";
        case TokenType::String:
            return "TOKEN_STRING";
        case TokenType::EndOfFile:
            return "TOKEN_EOF";
    }

    throw std::runtime_error("token type desconhecido");
}

}  // namespace spike
