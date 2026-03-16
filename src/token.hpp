#ifndef SPIKE_TOKEN_HPP
#define SPIKE_TOKEN_HPP

#include <cstddef>
#include <string>

#include "token_type.hpp"

struct Token {
    TokenType type;
    std::string lexeme;
    std::size_t line;
    std::size_t column;
};

std::string escape_lexeme(const std::string& lexeme);

#endif
