#ifndef SPIKE_TOKEN_HPP
#define SPIKE_TOKEN_HPP

#include <string>

namespace spike
{

    enum class TokenType
    {
        // Literais
        NUMBER,     // 42, 3.14
        STRING,     // "ola mundo"
        IDENTIFIER, // x, nome, contador

        // Keywords - estrutura do programa
        ALGORITMO,
        FIM_ALGORITMO,
        INICIO,
        VAR,

        // Keywords - tipos de dados
        INTEIRO,
        REAL,
        CARACTERE,
        LOGICO,

        // Keywords - controle de fluxo
        SE,
        ENTAO,
        SENAO,
        FIM_SE,
        ENQUANTO,
        FACA,
        FIM_ENQUANTO,
        PARA,
        DE,
        ATE,
        PASSO,
        FIM_PARA,
        REPITA,
        ATE_QUE, // usado no repita...ate

        // Keywords - I/O
        ESCREVA,
        ESCREVAL,
        LEIA,

        // Keywords - valores logicos
        VERDADEIRO,
        FALSO,

        // Keywords - operadores logicos como palavras
        E,
        OU,
        NAO,
        DIV,
        MOD,

        // Operadores aritmeticos
        PLUS,  // +
        MINUS, // -
        STAR,  // *
        SLASH, // /
        CARET,

        // Operadores de comparacao
        EQUAL,      // =
        NOT_EQUAL,  // <>
        LESS,       // <
        GREATER,    // >
        LESS_EQ,    // <=
        GREATER_EQ, // >=

        // Atribuicao
        ARROW,  // <-
        ASSIGN, // := (alternativa)

        // Pontuacao
        LPAREN, // (
        RPAREN, // )
        COMMA,  // ,
        COLON,  // :
        DOT,    // .

        // Especiais
        END_OF_FILE,
        UNKNOWN, // token nao reconhecido
    };

    struct Token
    {
        TokenType type;
        std::string lexeme;
        int line = 1;
        int col = 1;
    };

    // Returns the textual name of a TokenType (e.g. "NUMBER", "ALGORITMO").
    std::string token_type_to_string(TokenType t);

} // namespace spike

#endif // SPIKE_TOKEN_HPP
