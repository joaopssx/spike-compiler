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

} // namespace

Lexer::Lexer(const std::string& source, const std::string& filename)
    : source_(source), filename_(filename),
      current_(0), line_(1), col_(1),
      token_start_line_(1), token_start_col_(1),
      had_error_(false) {}

bool Lexer::had_error() const { return had_error_; }
const std::vector<std::string>& Lexer::errors() const { return errors_; }

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

void Lexer::add_error(const std::string& msg) {
    had_error_ = true;
    errors_.push_back(filename_ + ":" + std::to_string(line_) + ":" +
                      std::to_string(col_) + ": " + msg);
}

Token Lexer::scan_identifier_or_keyword() {
    const int start = current_;
    while (!is_at_end() && is_alphanumeric(peek())) {
        advance();
    }
    const std::string lexeme = source_.substr(
        static_cast<std::size_t>(start),
        static_cast<std::size_t>(current_ - start));

    const TokenType type = keyword_type(to_lower_ascii(lexeme));
    return make_token(type, lexeme);
}

Token Lexer::scan_number() {
    const int start = current_;
    while (!is_at_end() && is_digit(peek())) {
        advance();
    }

    // Parte decimal — só consome o '.' se houver dígito imediatamente depois.
    if (peek() == '.' && is_digit(peek_next())) {
        advance(); // consome o '.'
        while (!is_at_end() && is_digit(peek())) {
            advance();
        }
    } else if (peek() == '.') {
        // '.' sem dígito depois: consome o ponto, reporta erro, segue.
        advance();
        add_error("Número malformado: '.' sem dígito decimal");
    }

    // Tentativa de notação inválida como "3.4.5": o segundo '.' (com dígito)
    // dispararia um erro extra; aqui só detectamos o caso patológico.
    if (peek() == '.' && is_digit(peek_next())) {
        add_error("Número malformado: múltiplos pontos decimais");
        advance(); // consome o '.'
        while (!is_at_end() && is_digit(peek())) {
            advance();
        }
    }

    const std::string lexeme = source_.substr(
        static_cast<std::size_t>(start),
        static_cast<std::size_t>(current_ - start));
    return make_token(TokenType::NUMBER, lexeme);
}

Token Lexer::scan_string() {
    advance(); // consome a aspas de abertura

    std::string value;
    while (!is_at_end() && peek() != '"' && peek() != '\n') {
        char c = advance();
        if (c == '\\' && !is_at_end()) {
            const char next = advance();
            switch (next) {
                case '"':  value.push_back('"');  break;
                case '\\': value.push_back('\\'); break;
                case 'n':  value.push_back('\n'); break;
                case 't':  value.push_back('\t'); break;
                default:
                    // Escape desconhecido: mantém os dois chars literalmente.
                    value.push_back('\\');
                    value.push_back(next);
                    break;
            }
        } else {
            value.push_back(c);
        }
    }

    if (peek() != '"') {
        // EOF ou '\n' antes de fechar.
        add_error("String não fechada na linha " +
                  std::to_string(token_start_line_));
        return make_token(TokenType::STRING, value);
    }

    advance(); // consome a aspas de fechamento
    return make_token(TokenType::STRING, value);
}

TokenType Lexer::keyword_type(const std::string& word) const {
    static const std::unordered_map<std::string, TokenType> kKeywords = {
        {"algoritmo",     TokenType::ALGORITMO},
        {"fimalgoritmo",  TokenType::FIM_ALGORITMO},
        {"inicio",        TokenType::INICIO},
        {"var",           TokenType::VAR},

        {"inteiro",       TokenType::INTEIRO},
        {"real",          TokenType::REAL},
        {"caractere",     TokenType::CARACTERE},
        {"caracter",      TokenType::CARACTERE},  // dialeto VisuAlg
        {"logico",        TokenType::LOGICO},

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

        {"escreva",       TokenType::ESCREVA},
        {"escreval",      TokenType::ESCREVAL},
        {"leia",          TokenType::LEIA},

        {"verdadeiro",    TokenType::VERDADEIRO},
        {"falso",         TokenType::FALSO},

        {"e",             TokenType::E},
        {"ou",            TokenType::OU},
        {"nao",           TokenType::NAO},
    };

    const auto it = kKeywords.find(word);
    if (it != kKeywords.end()) return it->second;
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

    if (is_alpha(c)) {
        out = scan_identifier_or_keyword();
        return true;
    }

    if (is_digit(c)) {
        out = scan_number();
        return true;
    }

    if (c == '"') {
        out = scan_string();
        return true;
    }

    // Comentário de linha: // ... \n
    if (c == '/' && peek_next() == '/') {
        while (!is_at_end() && peek() != '\n') {
            advance();
        }
        return false;
    }

    // Comentário de bloco: { ... }
    if (c == '{') {
        const int open_line = line_;
        advance(); // consome '{'
        while (!is_at_end() && peek() != '}') {
            if (peek() == '\n') {
                advance();
                ++line_;
                col_ = 1;
            } else {
                advance();
            }
        }
        if (is_at_end()) {
            add_error("Comentário não fechado (aberto na linha " +
                      std::to_string(open_line) + ")");
            return false;
        }
        advance(); // consome '}'
        return false;
    }

    // Operadores e pontuação de um caractere.
    switch (c) {
        case '+': advance(); out = make_token(TokenType::PLUS,   "+"); return true;
        case '-': advance(); out = make_token(TokenType::MINUS,  "-"); return true;
        case '*': advance(); out = make_token(TokenType::STAR,   "*"); return true;
        case '/': advance(); out = make_token(TokenType::SLASH,  "/"); return true;
        case '(': advance(); out = make_token(TokenType::LPAREN, "("); return true;
        case ')': advance(); out = make_token(TokenType::RPAREN, ")"); return true;
        case ',': advance(); out = make_token(TokenType::COMMA,  ","); return true;
        case '.': advance(); out = make_token(TokenType::DOT,    "."); return true;
        case '=': advance(); out = make_token(TokenType::EQUAL,  "="); return true;
        default: break;
    }

    // Operadores que podem ter um ou dois caracteres.
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

    // Caractere desconhecido — consome um char pra avançar e reporta.
    advance();
    add_error(std::string("Caractere desconhecido '") + c +
              "' na linha " + std::to_string(token_start_line_) +
              ", coluna " + std::to_string(token_start_col_));
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
