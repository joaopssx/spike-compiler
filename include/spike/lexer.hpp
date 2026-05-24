#ifndef SPIKE_LEXER_HPP
#define SPIKE_LEXER_HPP

#include "spike/token.hpp"

#include <string>
#include <vector>

namespace spike {

class Lexer {
public:
    Lexer(const std::string& source, const std::string& filename);
    std::vector<Token> tokenize();
    bool had_error() const;
    const std::vector<std::string>& errors() const;

private:
    std::string source_;
    std::string filename_;
    int current_;
    int line_;
    int col_;
    int token_start_line_;
    int token_start_col_;
    bool had_error_;
    std::vector<std::string> errors_;

    bool scan_one_token(Token& out);

    // Navegação
    char peek() const;
    char peek_next() const;
    char advance();
    bool is_at_end() const;
    bool match(char expected);

    // Classificação de caracteres
    bool is_digit(char c) const;
    bool is_alpha(char c) const;
    bool is_alphanumeric(char c) const;

    // Criação de tokens
    Token make_token(TokenType type, const std::string& lexeme) const;
    void add_error(const std::string& msg);

    // Scanners específicos
    Token scan_identifier_or_keyword();
    Token scan_number();
    Token scan_string();

    // Keywords case-insensitive: "Se" = "se" = "SE"
    TokenType keyword_type(const std::string& word) const;
};

} // namespace spike

#endif // SPIKE_LEXER_HPP
