#ifndef SPIKE_PARSER_INTERNAL_HPP
#define SPIKE_PARSER_INTERNAL_HPP

#include "spike/ast_enums.hpp"
#include "spike/token.hpp"

namespace spike::parser_detail {

inline TypeKind type_kind_for(TokenType t) {
    switch (t) {
        case TokenType::INTEIRO:   return TypeKind::Integer;
        case TokenType::REAL:      return TypeKind::Real;
        case TokenType::CARACTERE: return TypeKind::String;
        case TokenType::LOGICO:    return TypeKind::Bool;
        default:                   return TypeKind::Invalid;
    }
}

inline bool is_type_keyword(TokenType t) {
    return t == TokenType::INTEIRO || t == TokenType::REAL || t == TokenType::CARACTERE ||
           t == TokenType::LOGICO;
}

} // namespace spike::parser_detail

#endif // SPIKE_PARSER_INTERNAL_HPP
