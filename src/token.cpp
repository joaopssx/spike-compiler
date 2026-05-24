#include "spike/token.hpp"

namespace spike {

std::string token_type_to_string(TokenType t) {
    switch (t) {
        // Literais
        case TokenType::NUMBER:        return "NUMBER";
        case TokenType::STRING:        return "STRING";
        case TokenType::IDENTIFIER:    return "IDENTIFIER";

        // Keywords — estrutura do programa
        case TokenType::ALGORITMO:     return "ALGORITMO";
        case TokenType::FIM_ALGORITMO: return "FIM_ALGORITMO";
        case TokenType::INICIO:        return "INICIO";
        case TokenType::VAR:           return "VAR";

        // Keywords — tipos de dados
        case TokenType::INTEIRO:       return "INTEIRO";
        case TokenType::REAL:          return "REAL";
        case TokenType::CARACTERE:     return "CARACTERE";
        case TokenType::LOGICO:        return "LOGICO";

        // Keywords — controle de fluxo
        case TokenType::SE:            return "SE";
        case TokenType::ENTAO:         return "ENTAO";
        case TokenType::SENAO:         return "SENAO";
        case TokenType::FIM_SE:        return "FIM_SE";
        case TokenType::ENQUANTO:      return "ENQUANTO";
        case TokenType::FACA:          return "FACA";
        case TokenType::FIM_ENQUANTO:  return "FIM_ENQUANTO";
        case TokenType::PARA:          return "PARA";
        case TokenType::DE:            return "DE";
        case TokenType::ATE:           return "ATE";
        case TokenType::PASSO:         return "PASSO";
        case TokenType::FIM_PARA:      return "FIM_PARA";
        case TokenType::REPITA:        return "REPITA";
        case TokenType::ATE_QUE:       return "ATE_QUE";

        // Keywords — I/O
        case TokenType::ESCREVA:       return "ESCREVA";
        case TokenType::ESCREVAL:      return "ESCREVAL";
        case TokenType::LEIA:          return "LEIA";

        // Keywords — valores lógicos
        case TokenType::VERDADEIRO:    return "VERDADEIRO";
        case TokenType::FALSO:         return "FALSO";

        // Keywords — operadores lógicos como palavras
        case TokenType::E:             return "E";
        case TokenType::OU:            return "OU";
        case TokenType::NAO:           return "NAO";

        // Operadores aritméticos
        case TokenType::PLUS:          return "PLUS";
        case TokenType::MINUS:         return "MINUS";
        case TokenType::STAR:          return "STAR";
        case TokenType::SLASH:         return "SLASH";

        // Operadores de comparação
        case TokenType::EQUAL:         return "EQUAL";
        case TokenType::NOT_EQUAL:     return "NOT_EQUAL";
        case TokenType::LESS:          return "LESS";
        case TokenType::GREATER:       return "GREATER";
        case TokenType::LESS_EQ:       return "LESS_EQ";
        case TokenType::GREATER_EQ:    return "GREATER_EQ";

        // Atribuição
        case TokenType::ARROW:         return "ARROW";
        case TokenType::ASSIGN:        return "ASSIGN";

        // Pontuação
        case TokenType::LPAREN:        return "LPAREN";
        case TokenType::RPAREN:        return "RPAREN";
        case TokenType::COMMA:         return "COMMA";
        case TokenType::COLON:         return "COLON";
        case TokenType::DOT:           return "DOT";

        // Especiais
        case TokenType::END_OF_FILE:   return "END_OF_FILE";
        case TokenType::UNKNOWN:       return "UNKNOWN";
    }
    return "UNKNOWN";
}

} // namespace spike
