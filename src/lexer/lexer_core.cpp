#include "spike/lexer.hpp"
#include "lexer/lexer_hints.hpp"

#include <cctype>
#include <string>
#include <vector>

namespace spike {

using namespace lexer_hints;

Lexer::Lexer(const std::string& source, const std::string& filename)
    : source_(source), filename_(filename),
      current_(0), line_(1), col_(1),
      token_start_line_(1), token_start_col_(1),
      diags_(filename, source) {}

bool Lexer::had_error() const {
    return diags_.has_errors();
}

const Diagnostics& Lexer::diagnostics() const {
    return diags_;
}

bool Lexer::is_at_end() const {
    return current_ >= static_cast<int>(source_.size());
}

char Lexer::peek() const {
    if (is_at_end()) return '\0';
    return source_[static_cast<std::size_t>(current_)];
}

char Lexer::peek_next() const {
    if (current_ + 1 >= static_cast<int>(source_.size())) return '\0';
    return source_[static_cast<std::size_t>(current_ + 1)];
}

char Lexer::advance() {
    const char c = source_[static_cast<std::size_t>(current_)];
    ++current_;
    ++col_;
    return c;
}

bool Lexer::match(char expected) {
    if (is_at_end()) return false;
    if (peek() != expected) return false;
    advance();
    return true;
}

bool Lexer::is_digit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::is_alpha(char c) const {
    const unsigned char u = static_cast<unsigned char>(c);
    return std::isalpha(u) || c == '_';
}

bool Lexer::is_alphanumeric(char c) const {
    return is_alpha(c) || is_digit(c);
}

Token Lexer::make_token(TokenType type, const std::string& lexeme) const {
    Token t;
    t.type = type;
    t.lexeme = lexeme;
    t.line = token_start_line_;
    t.col = token_start_col_;
    return t;
}

void Lexer::add_error(const std::string& code, const std::string& message,
                      const std::string& hint) {
    diags_.error(code, message, token_start_line_, token_start_col_, hint);
}

bool Lexer::scan_one_token(Token& out) {
    token_start_line_ = line_;
    token_start_col_ = col_;

    const char c = peek();

    if (c == ' ' || c == '\t' || c == '\r') {
        advance();
        return false;
    }
    if (c == '\n') {
        advance();
        ++line_;
        col_ = 1;
        return false;
    }

    if (is_alpha(c))  { out = scan_identifier_or_keyword(); return true; }
    if (is_digit(c))  { out = scan_number();                 return true; }
    if (c == '"')     { out = scan_string();                 return true; }

    // Comentario de linha
    if (c == '/' && peek_next() == '/') {
        while (!is_at_end() && peek() != '\n') advance();
        return false;
    }

    // Comentario de bloco
    if (c == '{') {
        advance();
        while (!is_at_end() && peek() != '}') {
            if (peek() == '\n') { advance(); ++line_; col_ = 1; }
            else                { advance(); }
        }
        if (is_at_end()) {
            add_error("E004", "comentário de bloco não fechado", kE004Hint);
            return false;
        }
        advance();
        return false;
    }

    switch (c) {
        case '+': advance(); out = make_token(TokenType::PLUS,   "+"); return true;
        case '-': advance(); out = make_token(TokenType::MINUS,  "-"); return true;
        case '*': advance(); out = make_token(TokenType::STAR,   "*"); return true;
        case '/': advance(); out = make_token(TokenType::SLASH,  "/"); return true;
        case '^': advance(); out = make_token(TokenType::CARET,  "^"); return true;
        case '(': advance(); out = make_token(TokenType::LPAREN, "("); return true;
        case ')': advance(); out = make_token(TokenType::RPAREN, ")"); return true;
        case ',': advance(); out = make_token(TokenType::COMMA,  ","); return true;
        case '.':
            advance();
            if (match('.')) {
                out = make_token(TokenType::DOTDOT, "..");
            } else {
                out = make_token(TokenType::DOT, ".");
            }
            return true;
        case '[': advance(); out = make_token(TokenType::LBRACKET, "["); return true;
        case ']': advance(); out = make_token(TokenType::RBRACKET, "]"); return true;
        case '=': advance(); out = make_token(TokenType::EQUAL,  "="); return true;
        default: break;
    }

    if (c == '<') {
        advance();
        if (match('-')) { out = make_token(TokenType::ARROW,     "<-"); return true; }
        if (match('=')) { out = make_token(TokenType::LESS_EQ,   "<="); return true; }
        if (match('>')) { out = make_token(TokenType::NOT_EQUAL, "<>"); return true; }
        out = make_token(TokenType::LESS, "<");
        return true;
    }
    if (c == '>') {
        advance();
        if (match('=')) { out = make_token(TokenType::GREATER_EQ, ">="); return true; }
        out = make_token(TokenType::GREATER, ">");
        return true;
    }
    if (c == ':') {
        advance();
        if (match('=')) { out = make_token(TokenType::ASSIGN, ":="); return true; }
        out = make_token(TokenType::COLON, ":");
        return true;
    }

    advance();
    add_error("E001",
              std::string("caractere desconhecido '") + c + "'",
              kE001Hint);
    out = make_token(TokenType::UNKNOWN, std::string(1, c));
    return true;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (!is_at_end()) {
        Token t;
        if (scan_one_token(t)) tokens.push_back(t);
    }
    token_start_line_ = line_;
    token_start_col_ = col_;
    tokens.push_back(make_token(TokenType::END_OF_FILE, ""));
    return tokens;
}

} // namespace spike
