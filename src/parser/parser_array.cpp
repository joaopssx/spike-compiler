#include "spike/parser.hpp"

#include <stdexcept>
#include <string>
#include <utility>

namespace spike {

namespace {

constexpr const char* kHintVetor = "formato: nome : vetor[1..10] de tipo.";

} // namespace

ExprPtr Parser::parse_name_reference(const Token& name) {
    if (!match(TokenType::LBRACKET)) return make_expr(VarExpr{name.lexeme, name.line, name.col});
    IndexExpr index;
    index.name = name.lexeme;
    index.line = name.line;
    index.col = name.col;
    do {
        index.indices.push_back(parse_expr());
    } while (match(TokenType::COMMA));
    if (!match(TokenType::RBRACKET)) {
        const Token& t = peek();
        diags_.error("E825", "']' esperado depois do índice de '" + name.lexeme + "'", t.line,
                     t.col, "feche o índice com ']': " + name.lexeme + "[i].");
    }
    return make_expr(std::move(index));
}

bool Parser::parse_array_bound(ArrayBound& bound) {
    const Token& start = peek();
    bound.line = start.line;
    bound.col = start.col;
    bound.negative = match(TokenType::MINUS);
    const Token& t = peek();
    if (check(TokenType::NUMBER) && t.lexeme.find('.') == std::string::npos) {
        try {
            bound.value = std::stoll(t.lexeme);
        } catch (const std::out_of_range&) {
            diags_.error("E821", "limite do vetor grande demais", t.line, t.col, kHintVetor);
            return false;
        }
        if (bound.negative) bound.value = -bound.value;
        advance();
        return true;
    }
    if (check(TokenType::IDENTIFIER)) {
        bound.constant = t.lexeme;
        advance();
        return true;
    }
    diags_.error("E821", "limite do vetor esperado (um número inteiro ou uma constante)",
                 t.line, t.col, kHintVetor);
    return false;
}

bool Parser::parse_array_type(std::vector<ArrayDim>& dims) {
    if (!match(TokenType::LBRACKET)) {
        const Token& t = peek();
        diags_.error("E820", "'[' esperado depois de 'vetor'", t.line, t.col, kHintVetor);
        return false;
    }
    ArrayDim dim;
    if (!parse_array_bound(dim.low)) return false;
    if (!match(TokenType::DOTDOT)) {
        const Token& t = peek();
        diags_.error("E822", "'..' esperado entre os limites do vetor", t.line, t.col,
                     kHintVetor);
        return false;
    }
    if (!parse_array_bound(dim.high)) return false;
    dims.push_back(dim);
    if (!match(TokenType::RBRACKET)) {
        const Token& t = peek();
        diags_.error("E823", "']' esperado depois dos limites do vetor", t.line, t.col,
                     kHintVetor);
        return false;
    }
    if (!match(TokenType::DE)) {
        const Token& t = peek();
        diags_.error("E824", "'de' esperado depois de ']'", t.line, t.col, kHintVetor);
        return false;
    }
    return true;
}

} // namespace spike
