#include "lexer.hpp"

#include <cctype>
#include <unordered_map>
#include <utility>

LexerError::LexerError(std::string message, const std::size_t line, const std::size_t column)
    : std::runtime_error(std::move(message)), line_(line), column_(column) {}

std::size_t LexerError::line() const noexcept {
    return line_;
}

std::size_t LexerError::column() const noexcept {
    return column_;
}

Lexer::Lexer(std::string source)
    : source_(std::move(source)), index_(0), line_(1), column_(1) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        skip_whitespace_and_comments();

        if (is_at_end()) {
            tokens.push_back(Token{TokenType::EndOfFile, "", line_, column_});
            return tokens;
        }

        const std::size_t start_index = index_;
        const std::size_t start_line = line_;
        const std::size_t start_column = column_;
        const char ch = advance();

        if (std::isalpha(static_cast<unsigned char>(ch)) != 0 || ch == '_') {
            tokens.push_back(
                make_identifier_or_keyword(start_index, start_line, start_column));
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(ch)) != 0) {
            tokens.push_back(make_number(start_index, start_line, start_column));
            continue;
        }

        switch (ch) {
            case '+':
                tokens.push_back(Token{TokenType::Plus, "+", start_line, start_column});
                break;
            case '-':
                tokens.push_back(Token{TokenType::Minus, "-", start_line, start_column});
                break;
            case '*':
                tokens.push_back(Token{TokenType::Star, "*", start_line, start_column});
                break;
            case '/':
                tokens.push_back(Token{TokenType::Slash, "/", start_line, start_column});
                break;
            case '(':
                tokens.push_back(Token{TokenType::LeftParen, "(", start_line, start_column});
                break;
            case ')':
                tokens.push_back(Token{TokenType::RightParen, ")", start_line, start_column});
                break;
            case ',':
                tokens.push_back(Token{TokenType::Comma, ",", start_line, start_column});
                break;
            case ':':
                tokens.push_back(Token{TokenType::Colon, ":", start_line, start_column});
                break;
            case '=':
                tokens.push_back(Token{TokenType::Equal, "=", start_line, start_column});
                break;
            case '<':
                if (peek() == '-') {
                    advance();
                    tokens.push_back(
                        Token{TokenType::Assign, "<-", start_line, start_column});
                } else if (peek() == '=') {
                    advance();
                    tokens.push_back(
                        Token{TokenType::LessEqual, "<=", start_line, start_column});
                } else {
                    tokens.push_back(Token{TokenType::Less, "<", start_line, start_column});
                }
                break;
            case '>':
                if (peek() == '=') {
                    advance();
                    tokens.push_back(
                        Token{TokenType::GreaterEqual, ">=", start_line, start_column});
                } else {
                    tokens.push_back(
                        Token{TokenType::Greater, ">", start_line, start_column});
                }
                break;
            case '"':
                tokens.push_back(make_string(start_line, start_column));
                break;
            default:
                throw_error(std::string("unexpected character: '") + ch + "'", start_line,
                            start_column);
        }
    }
}

bool Lexer::is_at_end() const {
    return index_ >= source_.size();
}

char Lexer::peek() const {
    if (is_at_end()) {
        return '\0';
    }

    return source_[index_];
}

char Lexer::peek_next() const {
    if (index_ + 1 >= source_.size()) {
        return '\0';
    }

    return source_[index_ + 1];
}

char Lexer::advance() {
    const char ch = source_[index_++];

    if (ch == '\n') {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }

    return ch;
}

void Lexer::skip_whitespace_and_comments() {
    while (!is_at_end()) {
        const char ch = peek();

        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
            advance();
            continue;
        }

        if (ch == '/' && peek_next() == '/') {
            while (!is_at_end() && peek() != '\n') {
                advance();
            }
            continue;
        }

        return;
    }
}

Token Lexer::make_identifier_or_keyword(const std::size_t start_index,
                                        const std::size_t start_line,
                                        const std::size_t start_column) {
    while (!is_at_end()) {
        const char ch = peek();
        const bool is_word = std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_';

        if (!is_word) {
            break;
        }

        advance();
    }

    const std::string lexeme = source_.substr(start_index, index_ - start_index);

    static const std::unordered_map<std::string, TokenType> keywords = {
        {"algoritmo", TokenType::KeywordAlgoritmo},
        {"var", TokenType::KeywordVar},
        {"inicio", TokenType::KeywordInicio},
        {"fimalgoritmo", TokenType::KeywordFimAlgoritmo},
        {"inteiro", TokenType::KeywordInteiro},
        {"real", TokenType::KeywordReal},
        {"texto", TokenType::KeywordTexto},
        {"leia", TokenType::KeywordLeia},
        {"escreva", TokenType::KeywordEscreva},
    };

    const auto keyword = keywords.find(lexeme);
    if (keyword != keywords.end()) {
        return Token{keyword->second, lexeme, start_line, start_column};
    }

    return Token{TokenType::Identifier, lexeme, start_line, start_column};
}

Token Lexer::make_number(const std::size_t start_index, const std::size_t start_line,
                         const std::size_t start_column) {
    while (std::isdigit(static_cast<unsigned char>(peek())) != 0) {
        advance();
    }

    TokenType type = TokenType::IntegerLiteral;

    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peek_next())) != 0) {
        type = TokenType::RealLiteral;
        advance();

        while (std::isdigit(static_cast<unsigned char>(peek())) != 0) {
            advance();
        }
    }

    const std::string lexeme = source_.substr(start_index, index_ - start_index);
    return Token{type, lexeme, start_line, start_column};
}

Token Lexer::make_string(const std::size_t start_line, const std::size_t start_column) {
    std::string value;

    while (!is_at_end()) {
        const char ch = advance();

        if (ch == '"') {
            return Token{TokenType::StringLiteral, value, start_line, start_column};
        }

        if (ch == '\n') {
            throw_error("unterminated string literal", start_line, start_column);
        }

        if (ch == '\\') {
            if (is_at_end()) {
                throw_error("unterminated string literal", start_line, start_column);
            }

            const char escaped = advance();
            switch (escaped) {
                case 'n':
                    value += '\n';
                    break;
                case 't':
                    value += '\t';
                    break;
                case 'r':
                    value += '\r';
                    break;
                case '\\':
                    value += '\\';
                    break;
                case '"':
                    value += '"';
                    break;
                default:
                    value += escaped;
                    break;
            }

            continue;
        }

        value += ch;
    }

    throw_error("unterminated string literal", start_line, start_column);
}

[[noreturn]] void Lexer::throw_error(const std::string& message, const std::size_t line,
                                     const std::size_t column) const {
    throw LexerError(message, line, column);
}
