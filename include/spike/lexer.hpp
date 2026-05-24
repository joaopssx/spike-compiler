#ifndef SPIKE_LEXER_HPP
#define SPIKE_LEXER_HPP

#include "spike/token.hpp"

#include <string>
#include <vector>

namespace spike {

class Lexer {
public:
    // Recebe o conteúdo do arquivo como string e o nome do arquivo (para mensagens de erro)
    Lexer(const std::string& source, const std::string& filename);

    // Faz a análise léxica completa e retorna todos os tokens
    // O último token é sempre END_OF_FILE
    std::vector<Token> tokenize();

    // Retorna true se houve algum erro durante a tokenização
    bool had_error() const;

    // Retorna as mensagens de erro encontradas
    const std::vector<std::string>& errors() const;

private:
    std::string source_;
    std::string filename_;
    int current_;   // índice do caractere atual
    int line_;      // linha atual (começa em 1)
    int col_;       // coluna atual (começa em 1)
    int token_start_line_;
    int token_start_col_;
    bool had_error_;
    std::vector<std::string> errors_;

    // Loop interno: lê um único token (ou consome whitespace).
    // Retorna true se um token foi emitido em `out`, false se só consumiu whitespace.
    bool scan_one_token(Token& out);

    // Helpers de navegação
    char peek() const;          // olha o char atual sem consumir
    char peek_next() const;     // olha o próximo char sem consumir
    char advance();             // consome e retorna o char atual
    bool is_at_end() const;
    bool match(char expected);  // consome se bater, retorna bool

    // Criação de tokens
    Token make_token(TokenType type, const std::string& lexeme) const;
    void add_error(const std::string& msg);

    // Scanners específicos
    Token scan_identifier_or_keyword();

    // Tabela de keywords (retorna o TokenType se for keyword, IDENTIFIER se não for)
    // Keywords são case-insensitive: "Se" = "se" = "SE"
    TokenType keyword_type(const std::string& word) const;
};

} // namespace spike

#endif // SPIKE_LEXER_HPP
