#include "spike/lexer.h"

#include <cctype>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

namespace spike {
namespace {

bool IsIdentifierStart(char value) {
    const unsigned char character = static_cast<unsigned char>(value);
    return std::isalpha(character) != 0 || value == '_';
}

bool IsIdentifierPart(char value) {
    const unsigned char character = static_cast<unsigned char>(value);
    return std::isalnum(character) != 0 || value == '_';
}

bool IsSupportedEscapeCharacter(char value) {
    return value == '"' || value == '\\' || value == 'n' || value == 't';
}

std::string ToLowerAscii(const std::string& text) {
    std::string lowered;
    lowered.reserve(text.size());

    for (char value : text) {
        lowered.push_back(
            static_cast<char>(std::tolower(static_cast<unsigned char>(value))));
    }

    return lowered;
}

const std::unordered_map<std::string, TokenType> kKeywords = {
    {"algoritmo", TokenType::Algoritmo},
    {"var", TokenType::Var},
    {"inicio", TokenType::Inicio},
    {"fimalgoritmo", TokenType::FimAlgoritmo},
    {"se", TokenType::Se},
    {"entao", TokenType::Entao},
    {"senao", TokenType::Senao},
    {"fimse", TokenType::FimSe},
    {"enquanto", TokenType::Enquanto},
    {"faca", TokenType::Faca},
    {"fimenquanto", TokenType::FimEnquanto},
    {"inteiro", TokenType::Inteiro},
    {"real", TokenType::Real},
    {"texto", TokenType::Texto},
    {"logico", TokenType::Logico},
    {"verdadeiro", TokenType::Verdadeiro},
    {"falso", TokenType::Falso},
    {"leia", TokenType::Leia},
    {"escreva", TokenType::Escreva},
    {"e", TokenType::E},
    {"ou", TokenType::Ou},
    {"nao", TokenType::Nao},
};

}  // namespace

Lexer::Lexer(std::string source, std::string source_name)
    : source_(std::move(source)),
      source_name_(std::move(source_name)),
      position_(0),
      line_(1),
      column_(1) {}

std::vector<Token> Lexer::Tokenize() {
    std::vector<Token> tokens;

    while (true) {
        SkipIgnoredText();
        if (IsAtEnd()) {
            break;
        }

        const std::size_t token_start = position_;
        const std::size_t token_line = line_;
        const std::size_t token_column = column_;
        const char current = Peek();

        if (IsIdentifierStart(current)) {
            tokens.push_back(ReadIdentifierOrKeyword());
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(current)) != 0) {
            tokens.push_back(ReadNumber());
            continue;
        }

        switch (current) {
            case '"':
                tokens.push_back(ReadString());
                break;
            case '+':
                Advance();
                tokens.push_back(
                    BuildToken(TokenType::Plus, token_start, token_line, token_column));
                break;
            case '-':
                Advance();
                tokens.push_back(BuildToken(TokenType::Minus, token_start, token_line,
                                            token_column));
                break;
            case '*':
                Advance();
                tokens.push_back(
                    BuildToken(TokenType::Star, token_start, token_line, token_column));
                break;
            case '/':
                Advance();
                tokens.push_back(
                    BuildToken(TokenType::Slash, token_start, token_line, token_column));
                break;
            case ':':
                Advance();
                tokens.push_back(BuildToken(TokenType::Colon, token_start, token_line,
                                            token_column));
                break;
            case ',':
                Advance();
                tokens.push_back(BuildToken(TokenType::Comma, token_start, token_line,
                                            token_column));
                break;
            case '(':
                Advance();
                tokens.push_back(BuildToken(TokenType::LeftParen, token_start, token_line,
                                            token_column));
                break;
            case ')':
                Advance();
                tokens.push_back(BuildToken(TokenType::RightParen, token_start, token_line,
                                            token_column));
                break;
            case '=':
                Advance();
                tokens.push_back(BuildToken(TokenType::Equal, token_start, token_line,
                                            token_column));
                break;
            case '<':
                Advance();
                if (Match('-')) {
                    tokens.push_back(BuildToken(TokenType::Assign, token_start, token_line,
                                                token_column));
                } else if (Match('=')) {
                    tokens.push_back(BuildToken(TokenType::LessEqual, token_start,
                                                token_line, token_column));
                } else if (Match('>')) {
                    tokens.push_back(BuildToken(TokenType::NotEqual, token_start,
                                                token_line, token_column));
                } else {
                    tokens.push_back(BuildToken(TokenType::Less, token_start, token_line,
                                                token_column));
                }
                break;
            case '>':
                Advance();
                if (Match('=')) {
                    tokens.push_back(BuildToken(TokenType::GreaterEqual, token_start,
                                                token_line, token_column));
                } else {
                    tokens.push_back(BuildToken(TokenType::Greater, token_start, token_line,
                                                token_column));
                }
                break;
            default:
                ThrowError(std::string("caractere inesperado: '") + current + "'",
                           token_line, token_column);
        }
    }

    tokens.push_back(Token{TokenType::EndOfFile, "", line_, column_});
    return tokens;
}

bool Lexer::IsAtEnd() const { return position_ >= source_.size(); }

char Lexer::Peek(std::size_t offset) const {
    const std::size_t index = position_ + offset;
    if (index >= source_.size()) {
        return '\0';
    }

    return source_[index];
}

char Lexer::Advance() {
    const char current = source_[position_];
    ++position_;

    if (current == '\n') {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }

    return current;
}

bool Lexer::Match(char expected) {
    if (IsAtEnd() || Peek() != expected) {
        return false;
    }

    Advance();
    return true;
}

void Lexer::SkipIgnoredText() {
    while (!IsAtEnd()) {
        const char current = Peek();
        if (current == ' ' || current == '\t' || current == '\r' || current == '\n') {
            Advance();
            continue;
        }

        if (current == '/' && Peek(1) == '/') {
            while (!IsAtEnd() && Peek() != '\n') {
                Advance();
            }

            continue;
        }

        break;
    }
}

Token Lexer::ReadIdentifierOrKeyword() {
    const std::size_t token_start = position_;
    const std::size_t token_line = line_;
    const std::size_t token_column = column_;

    while (IsIdentifierPart(Peek())) {
        Advance();
    }

    const std::string lexeme = source_.substr(token_start, position_ - token_start);
    const auto keyword = kKeywords.find(ToLowerAscii(lexeme));

    if (keyword != kKeywords.end()) {
        return Token{keyword->second, lexeme, token_line, token_column};
    }

    return Token{TokenType::Identifier, lexeme, token_line, token_column};
}

Token Lexer::ReadNumber() {
    const std::size_t token_start = position_;
    const std::size_t token_line = line_;
    const std::size_t token_column = column_;

    while (std::isdigit(static_cast<unsigned char>(Peek())) != 0) {
        Advance();
    }

    if (Peek() == '.' && std::isdigit(static_cast<unsigned char>(Peek(1))) != 0) {
        Advance();

        while (std::isdigit(static_cast<unsigned char>(Peek())) != 0) {
            Advance();
        }
    }

    return BuildToken(TokenType::Number, token_start, token_line, token_column);
}

Token Lexer::ReadString() {
    const std::size_t token_start = position_;
    const std::size_t token_line = line_;
    const std::size_t token_column = column_;

    Advance();

    while (!IsAtEnd() && Peek() != '"') {
        if (Peek() == '\n') {
            ThrowError("string nao terminada", token_line, token_column);
        }

        if (Peek() == '\\') {
            const std::size_t escape_line = line_;
            const std::size_t escape_column = column_;

            Advance();
            if (IsAtEnd() || Peek() == '\n') {
                ThrowError("string nao terminada", token_line, token_column);
            }

            if (!IsSupportedEscapeCharacter(Peek())) {
                ThrowError(std::string("sequencia de escape invalida: \\") + Peek(),
                           escape_line, escape_column);
            }

            Advance();
            continue;
        }

        Advance();
    }

    if (IsAtEnd()) {
        ThrowError("string nao terminada", token_line, token_column);
    }

    Advance();
    return BuildToken(TokenType::String, token_start, token_line, token_column);
}

Token Lexer::BuildToken(TokenType type, std::size_t start, std::size_t line,
                        std::size_t column) const {
    return Token{type, source_.substr(start, position_ - start), line, column};
}

void Lexer::ThrowError(const std::string& message, std::size_t line,
                       std::size_t column) const {
    const std::string line_text = GetLineText(line);
    std::string formatted_message = "Erro";
    if (!source_name_.empty()) {
        formatted_message += " em " + source_name_;
    }

    formatted_message += ": linha " + std::to_string(line) + ", coluna " +
                         std::to_string(column) + ": " + message;

    if (!line_text.empty()) {
        std::string caret_line(column > 1 ? column - 1 : 0, ' ');
        caret_line += '^';
        formatted_message += "\n  " + line_text + "\n  " + caret_line;
    }

    throw std::runtime_error(formatted_message);
}

std::string Lexer::GetLineText(std::size_t target_line) const {
    std::size_t current_line = 1;
    std::size_t line_start = 0;

    for (std::size_t index = 0; index <= source_.size(); ++index) {
        if (index == source_.size() || source_[index] == '\n') {
            if (current_line == target_line) {
                std::string line_text = source_.substr(line_start, index - line_start);
                if (!line_text.empty() && line_text.back() == '\r') {
                    line_text.pop_back();
                }

                return line_text;
            }

            ++current_line;
            line_start = index + 1;
        }
    }

    return "";
}

}  // namespace spike
