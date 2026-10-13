#include "spike/parser.hpp"

#include <string>
#include <utility>

namespace spike {

namespace {

BinaryOp binary_op_for(TokenType t) {
    switch (t) {
        case TokenType::PLUS:       return BinaryOp::Add;
        case TokenType::MINUS:      return BinaryOp::Sub;
        case TokenType::STAR:       return BinaryOp::Mul;
        case TokenType::SLASH:      return BinaryOp::Div;
        case TokenType::EQUAL:      return BinaryOp::Eq;
        case TokenType::NOT_EQUAL:  return BinaryOp::Ne;
        case TokenType::LESS:       return BinaryOp::Lt;
        case TokenType::GREATER:    return BinaryOp::Gt;
        case TokenType::LESS_EQ:    return BinaryOp::Le;
        case TokenType::GREATER_EQ: return BinaryOp::Ge;
        case TokenType::E:          return BinaryOp::And;
        case TokenType::OU:         return BinaryOp::Or;
        default:                    return BinaryOp::Add;
    }
}

} // namespace

// ─── Expressões (PROMPT 015) ────────────────────────────────────────────────

ExprPtr Parser::parse_expr() {
    return parse_or();
}

ExprPtr Parser::parse_or() {
    ExprPtr left = parse_and();
    while (check(TokenType::OU)) {
        Token op = advance();
        ExprPtr right = parse_and();
        left = make_expr(BinaryExpr{
            binary_op_for(op.type), std::move(left), std::move(right),
            op.line, op.col});
    }
    return left;
}

ExprPtr Parser::parse_and() {
    ExprPtr left = parse_not();
    while (check(TokenType::E)) {
        Token op = advance();
        ExprPtr right = parse_not();
        left = make_expr(BinaryExpr{
            binary_op_for(op.type), std::move(left), std::move(right),
            op.line, op.col});
    }
    return left;
}

ExprPtr Parser::parse_not() {
    if (check(TokenType::NAO)) {
        Token op = advance();
        ExprPtr operand = parse_not();  // recursivo: "nao nao x" funciona
        return make_expr(UnaryExpr{
            UnaryOp::Not, std::move(operand), op.line, op.col});
    }
    return parse_comparison();
}

ExprPtr Parser::parse_comparison() {
    ExprPtr left = parse_term();
    while (check(TokenType::EQUAL)      || check(TokenType::NOT_EQUAL) ||
           check(TokenType::LESS)       || check(TokenType::GREATER)   ||
           check(TokenType::LESS_EQ)    || check(TokenType::GREATER_EQ)) {
        Token op = advance();
        ExprPtr right = parse_term();
        left = make_expr(BinaryExpr{
            binary_op_for(op.type), std::move(left), std::move(right),
            op.line, op.col});
    }
    return left;
}

ExprPtr Parser::parse_term() {
    ExprPtr left = parse_factor();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        Token op = advance();
        ExprPtr right = parse_factor();
        left = make_expr(BinaryExpr{
            binary_op_for(op.type), std::move(left), std::move(right),
            op.line, op.col});
    }
    return left;
}

ExprPtr Parser::parse_factor() {
    ExprPtr left = parse_unary();
    while (check(TokenType::STAR) || check(TokenType::SLASH)) {
        Token op = advance();
        ExprPtr right = parse_unary();
        left = make_expr(BinaryExpr{
            binary_op_for(op.type), std::move(left), std::move(right),
            op.line, op.col});
    }
    return left;
}

ExprPtr Parser::parse_unary() {
    if (check(TokenType::MINUS)) {
        Token op = advance();
        ExprPtr operand = parse_unary();  // recursivo: "--x" funciona
        return make_expr(UnaryExpr{
            UnaryOp::Neg, std::move(operand), op.line, op.col});
    }
    return parse_primary();
}

ExprPtr Parser::parse_primary() {
    Token tok = peek();  // cópia — vamos avançar abaixo

    if (check(TokenType::NUMBER)) {
        advance();
        const bool has_dot = tok.lexeme.find('.') != std::string::npos;
        // O lexer já validou: stod é seguro.
        const double value = std::stod(tok.lexeme);
        return make_expr(NumberExpr{value, !has_dot, tok.line, tok.col});
    }
    if (check(TokenType::STRING)) {
        advance();
        return make_expr(StringExpr{tok.lexeme, tok.line, tok.col});
    }
    if (check(TokenType::VERDADEIRO)) {
        advance();
        return make_expr(BoolExpr{true, tok.line, tok.col});
    }
    if (check(TokenType::FALSO)) {
        advance();
        return make_expr(BoolExpr{false, tok.line, tok.col});
    }
    if (check(TokenType::IDENTIFIER)) {
        advance();
        return make_expr(VarExpr{tok.lexeme, tok.line, tok.col});
    }
    if (check(TokenType::LPAREN)) {
        advance();
        ExprPtr inner = parse_expr();
        if (!match(TokenType::RPAREN)) {
            const Token& cur = peek();
            diags_.error("E200", "')' esperado",
                         cur.line, cur.col,
                         "feche o parêntese antes de continuar a expressão.");
        }
        return inner;
    }

    // Token inesperado. Emite E201 e CONSOME 1 token (a menos que seja EOF)
    // pra evitar loop infinito em chamadores que iteram esperando progresso.
    diags_.error("E201", "expressão esperada",
                 tok.line, tok.col,
                 "uma expressão pode ser número, string, variável, ou (expressão).");
    if (!is_at_end()) advance();
    return make_expr(NumberExpr{0.0, true, tok.line, tok.col});
}

} // namespace spike
