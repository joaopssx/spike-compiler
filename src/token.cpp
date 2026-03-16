#include "token.hpp"

#include <string_view>

std::string_view token_type_to_string(const TokenType type) {
    switch (type) {
        case TokenType::KeywordAlgoritmo:
            return "KEYWORD_ALGORITMO";
        case TokenType::KeywordVar:
            return "KEYWORD_VAR";
        case TokenType::KeywordInicio:
            return "KEYWORD_INICIO";
        case TokenType::KeywordFimAlgoritmo:
            return "KEYWORD_FIMALGORITMO";
        case TokenType::KeywordInteiro:
            return "KEYWORD_INTEIRO";
        case TokenType::KeywordReal:
            return "KEYWORD_REAL";
        case TokenType::KeywordTexto:
            return "KEYWORD_TEXTO";
        case TokenType::KeywordLeia:
            return "KEYWORD_LEIA";
        case TokenType::KeywordEscreva:
            return "KEYWORD_ESCREVA";
        case TokenType::Identifier:
            return "IDENTIFIER";
        case TokenType::IntegerLiteral:
            return "INTEGER_LITERAL";
        case TokenType::RealLiteral:
            return "REAL_LITERAL";
        case TokenType::StringLiteral:
            return "STRING_LITERAL";
        case TokenType::Plus:
            return "PLUS";
        case TokenType::Minus:
            return "MINUS";
        case TokenType::Star:
            return "STAR";
        case TokenType::Slash:
            return "SLASH";
        case TokenType::Assign:
            return "ASSIGN";
        case TokenType::Equal:
            return "EQUAL";
        case TokenType::Less:
            return "LESS";
        case TokenType::Greater:
            return "GREATER";
        case TokenType::LessEqual:
            return "LESS_EQUAL";
        case TokenType::GreaterEqual:
            return "GREATER_EQUAL";
        case TokenType::LeftParen:
            return "LEFT_PAREN";
        case TokenType::RightParen:
            return "RIGHT_PAREN";
        case TokenType::Comma:
            return "COMMA";
        case TokenType::Colon:
            return "COLON";
        case TokenType::EndOfFile:
            return "EOF";
    }

    return "UNKNOWN";
}

std::string escape_lexeme(const std::string& lexeme) {
    std::string escaped;
    escaped.reserve(lexeme.size());

    for (const char ch : lexeme) {
        switch (ch) {
            case '\\':
                escaped += "\\\\";
                break;
            case '"':
                escaped += "\\\"";
                break;
            case '\n':
                escaped += "\\n";
                break;
            case '\r':
                escaped += "\\r";
                break;
            case '\t':
                escaped += "\\t";
                break;
            default:
                escaped += ch;
                break;
        }
    }

    return escaped;
}
