#ifndef SPIKE_LEXER_HPP
#define SPIKE_LEXER_HPP

#include "spike/diagnostics.hpp"
#include "spike/token.hpp"

#include <string>
#include <vector>

namespace spike {

class Lexer {
public:
    // O `source` é também passado pro Diagnostics, então as mensagens podem
    // mostrar a linha do código onde o erro aconteceu.
    Lexer(const std::string& source, const std::string& filename);

    std::vector<Token> tokenize();

    bool had_error() const;
    const Diagnostics& diagnostics() const;

private:
    std::string source_;
    std::string filename_;
    int current_;
    int line_;
    int col_;
    int token_start_line_;
    int token_start_col_;
    Diagnostics diags_;

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

    // Reporta erro na posição inicial do token corrente.
    void add_error(const std::string& code, const std::string& message,
                   const std::string& hint);

    // Scanners específicos
    Token scan_identifier_or_keyword();
    Token scan_number();
    Token scan_string();

    // Keywords case-insensitive: "Se" = "se" = "SE"
    TokenType keyword_type(const std::string& word) const;
};

} // namespace spike

#endif // SPIKE_LEXER_HPP
