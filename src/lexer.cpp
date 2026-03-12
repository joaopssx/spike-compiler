#include "lexer.hpp"

#include <cctype>
#include <string_view>
#include <unordered_map>

#include "utils.hpp"

namespace portugol {

namespace {

const std::unordered_map<std::string, TokenType> kKeywords {
    {"algoritmo", TokenType::Algoritmo},
    {"var", TokenType::Var},
    {"inicio", TokenType::Inicio},
    {"fimalgoritmo", TokenType::FimAlgoritmo},
    {"inteiro", TokenType::Inteiro},
    {"real", TokenType::Real},
    {"logico", TokenType::Logico},
    {"caractere", TokenType::Caractere},
    {"escreva", TokenType::Escreva},
    {"leia", TokenType::Leia},
    {"se", TokenType::Se},
    {"entao", TokenType::Entao},
    {"senao", TokenType::Senao},
    {"fimse", TokenType::FimSe},
    {"enquanto", TokenType::Enquanto},
    {"faca", TokenType::Faca},
    {"fimenquanto", TokenType::FimEnquanto},
};

} // namespace

Lexer::Lexer(std::string source)
    : source_(std::move(source)) {
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        skipIgnored();
        if (isAtEnd()) {
            tokens.push_back(Token {TokenType::EndOfFile, "", SourceLocation {line_, column_, current_}});
            break;
        }

        tokens.push_back(scanToken());
    }

    return tokens;
}

bool Lexer::isAtEnd() const {
    return current_ >= source_.size();
}

char Lexer::peek(std::size_t offset) const {
    const std::size_t index = current_ + offset;
    if (index >= source_.size()) {
        return '\0';
    }

    return source_[index];
}

char Lexer::advance() {
    const char currentChar = source_[current_++];
    if (currentChar == '\n') {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }

    return currentChar;
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source_[current_] != expected) {
        return false;
    }

    advance();
    return true;
}

void Lexer::skipIgnored() {
    while (!isAtEnd()) {
        const char currentChar = peek();
        if (std::isspace(static_cast<unsigned char>(currentChar)) != 0) {
            advance();
            continue;
        }

        if (currentChar == '/' && peek(1) == '/') {
            while (!isAtEnd() && peek() != '\n') {
                advance();
            }
            continue;
        }

        break;
    }
}

Token Lexer::scanToken() {
    const SourceLocation startLocation {line_, column_, current_};
    const std::size_t startOffset = current_;
    const char currentChar = advance();

    switch (currentChar) {
    case '(':
        return makeToken(TokenType::LParen, startOffset, startLocation);
    case ')':
        return makeToken(TokenType::RParen, startOffset, startLocation);
    case ',':
        return makeToken(TokenType::Comma, startOffset, startLocation);
    case ':':
        return makeToken(TokenType::Colon, startOffset, startLocation);
    case '+':
        return makeToken(TokenType::Plus, startOffset, startLocation);
    case '-':
        return makeToken(TokenType::Minus, startOffset, startLocation);
    case '*':
        return makeToken(TokenType::Star, startOffset, startLocation);
    case '/':
        return makeToken(TokenType::Slash, startOffset, startLocation);
    case '<':
        if (match('-')) {
            return makeToken(TokenType::Assign, startOffset, startLocation);
        }
        if (match('=')) {
            return makeToken(TokenType::LessEqual, startOffset, startLocation);
        }
        if (match('>')) {
            return makeToken(TokenType::NotEqual, startOffset, startLocation);
        }
        return makeToken(TokenType::Less, startOffset, startLocation);
    case '>':
        if (match('=')) {
            return makeToken(TokenType::GreaterEqual, startOffset, startLocation);
        }
        return makeToken(TokenType::Greater, startOffset, startLocation);
    case '=':
        return makeToken(TokenType::Equal, startOffset, startLocation);
    case '"':
        return scanString(startLocation);
    default:
        break;
    }

    if (std::isdigit(static_cast<unsigned char>(currentChar)) != 0) {
        return scanNumber(startLocation, startOffset);
    }

    if (isIdentifierStart(currentChar)) {
        return scanIdentifier(startLocation, startOffset);
    }

    throw CompilerError("lexer", std::string("caractere inesperado: '") + currentChar + "'", startLocation);
}

Token Lexer::scanIdentifier(const SourceLocation& startLocation, std::size_t startOffset) {
    while (isIdentifierPart(peek())) {
        advance();
    }

    const std::string lexeme = source_.substr(startOffset, current_ - startOffset);
    const std::string lowered = toLowerAscii(lexeme);
    const auto it = kKeywords.find(lowered);
    if (it != kKeywords.end()) {
        return Token {it->second, lexeme, startLocation};
    }

    return Token {TokenType::Identifier, lexeme, startLocation};
}

Token Lexer::scanNumber(const SourceLocation& startLocation, std::size_t startOffset) {
    while (std::isdigit(static_cast<unsigned char>(peek())) != 0) {
        advance();
    }

    TokenType type = TokenType::NumberInt;
    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peek(1))) != 0) {
        type = TokenType::NumberReal;
        advance();
        while (std::isdigit(static_cast<unsigned char>(peek())) != 0) {
            advance();
        }
    }

    return makeToken(type, startOffset, startLocation);
}

Token Lexer::scanString(const SourceLocation& startLocation) {
    std::string value;

    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\n') {
            throw CompilerError("lexer", "literal de string nao pode quebrar linha", startLocation);
        }

        if (peek() == '\\') {
            advance();
            if (isAtEnd()) {
                break;
            }

            const char escaped = advance();
            switch (escaped) {
            case 'n':
                value.push_back('\n');
                break;
            case 't':
                value.push_back('\t');
                break;
            case '"':
                value.push_back('"');
                break;
            case '\\':
                value.push_back('\\');
                break;
            default:
                value.push_back(escaped);
                break;
            }
            continue;
        }

        value.push_back(advance());
    }

    if (isAtEnd()) {
        throw CompilerError("lexer", "literal de string nao foi finalizado", startLocation);
    }

    advance();
    return Token {TokenType::String, value, startLocation};
}

Token Lexer::makeToken(TokenType type, std::size_t startOffset, const SourceLocation& startLocation) const {
    return Token {type, source_.substr(startOffset, current_ - startOffset), startLocation};
}

bool Lexer::isIdentifierStart(char ch) {
    const unsigned char value = static_cast<unsigned char>(ch);
    return std::isalpha(value) != 0 || ch == '_';
}

bool Lexer::isIdentifierPart(char ch) {
    const unsigned char value = static_cast<unsigned char>(ch);
    return std::isalnum(value) != 0 || ch == '_';
}

std::string Lexer::toLowerAscii(std::string_view text) {
    std::string lowered;
    lowered.reserve(text.size());

    for (const char ch : text) {
        lowered.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
    }

    return lowered;
}

} // namespace portugol
