#include "spike/lexer.hpp"

#include <cctype>
#include <string>
#include <unordered_map>
#include <vector>

namespace spike {

namespace {

std::string to_lower_ascii(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) {
        out.push_back(static_cast<char>(std::tolower(c)));
    }
    return out;
}

bool is_alpha_char(char c) {
    const unsigned char u = static_cast<unsigned char>(c);
    return std::isalpha(u) || c == '_';
}

bool is_alnum_char(char c) {
    const unsigned char u = static_cast<unsigned char>(c);
    return std::isalnum(u) || c == '_';
}

} // namespace

Lexer::Lexer(const std::string& source, const std::string& filename)
    : source_(source),
      filename_(filename),
      current_(0),
      line_(1),
      col_(1),
      token_start_line_(1),
      token_start_col_(1),
      had_error_(false) {}

bool Lexer::had_error() const {
    return had_error_;
}

const std::vector<std::string>& Lexer::errors() const {
    return errors_;
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

Token Lexer::make_token(TokenType type, const std::string& lexeme) const {
    Token t;
    t.type = type;
    t.lexeme = lexeme;
    t.line = token_start_line_;
    t.col = token_start_col_;
    return t;
}

void Lexer::add_error(const std::string& msg) {
    had_error_ = true;
    errors_.push_back(filename_ + ":" + std::to_string(line_) + ":" +
                      std::to_string(col_) + ": " + msg);
}

Token Lexer::scan_identifier_or_keyword() {
    const int start = current_;
    while (!is_at_end() && is_alnum_char(peek())) {
        advance();
    }
    const std::string lexeme = source_.substr(
        static_cast<std::size_t>(start),
        static_cast<std::size_t>(current_ - start));

    const TokenType type = keyword_type(to_lower_ascii(lexeme));
    return make_token(type, lexeme);
}

TokenType Lexer::keyword_type(const std::string& word) const {
    static const std::unordered_map<std::string, TokenType> kKeywords = {
        // Estrutura do programa
        {"algoritmo",     TokenType::ALGORITMO},
        {"fimalgoritmo",  TokenType::FIM_ALGORITMO},
        {"inicio",        TokenType::INICIO},
        {"var",           TokenType::VAR},

        // Tipos de dados
        {"inteiro",       TokenType::INTEIRO},
        {"real",          TokenType::REAL},
        {"caractere",     TokenType::CARACTERE},
        {"caracter",      TokenType::CARACTERE},  // dialeto VisuAlg
        {"logico",        TokenType::LOGICO},

        // Controle de fluxo
        {"se",            TokenType::SE},
        {"entao",         TokenType::ENTAO},
        {"senao",         TokenType::SENAO},
        {"fimse",         TokenType::FIM_SE},
        {"enquanto",      TokenType::ENQUANTO},
        {"faca",          TokenType::FACA},
        {"fimenquanto",   TokenType::FIM_ENQUANTO},
        {"para",          TokenType::PARA},
        {"de",            TokenType::DE},
        {"ate",           TokenType::ATE},
        {"passo",         TokenType::PASSO},
        {"fimpara",       TokenType::FIM_PARA},
        {"repita",        TokenType::REPITA},

        // I/O
        {"escreva",       TokenType::ESCREVA},
        {"escreval",      TokenType::ESCREVAL},
        {"leia",          TokenType::LEIA},

        // Valores lógicos
        {"verdadeiro",    TokenType::VERDADEIRO},
        {"falso",         TokenType::FALSO},

        // Operadores lógicos
        {"e",             TokenType::E},
        {"ou",            TokenType::OU},
        {"nao",           TokenType::NAO},
    };

    const auto it = kKeywords.find(word);
    if (it != kKeywords.end()) {
        return it->second;
    }
    return TokenType::IDENTIFIER;
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

    if (is_alpha_char(c)) {
        out = scan_identifier_or_keyword();
        return true;
    }

    // Unknown character — consume one char to make forward progress.
    advance();
    add_error(std::string("caractere desconhecido: '") + c + "'");
    out = make_token(TokenType::UNKNOWN, std::string(1, c));
    return true;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (!is_at_end()) {
        Token t;
        if (scan_one_token(t)) {
            tokens.push_back(t);
        }
    }
    token_start_line_ = line_;
    token_start_col_ = col_;
    tokens.push_back(make_token(TokenType::END_OF_FILE, ""));
    return tokens;
}

} // namespace spike
