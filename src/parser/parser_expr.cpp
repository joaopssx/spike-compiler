#include "spike/parser.hpp"

#include <string>
#include <utility>
#include <variant>

namespace spike {

namespace {

BinaryOp binary_op_for(TokenType t) {
    switch (t) {
        case TokenType::PLUS:       return BinaryOp::Add;
        case TokenType::MINUS:      return BinaryOp::Sub;
        case TokenType::STAR:       return BinaryOp::Mul;
        case TokenType::SLASH:      return BinaryOp::Div;
        case TokenType::DIV:        return BinaryOp::IntDiv;
        case TokenType::MOD:        return BinaryOp::Mod;
        case TokenType::CARET:      return BinaryOp::Pow;
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

// Depois de um operando que faltou, o operador seguinte nao e lido: evita um
// segundo E201 no mesmo trecho.
bool is_error(const ExprPtr& expr) {
    return expr && std::holds_alternative<ErrorExpr>(expr->node);
}

} // namespace

// --- Expressoes -------------------------------------------------------------

ExprPtr Parser::parse_expr() {
    const NestingGuard guard(*this);
    return parse_or();
}

ExprPtr Parser::parse_or() {
    ExprPtr left = parse_and();
    while (!is_error(left) && check(TokenType::OU)) {
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
    while (!is_error(left) && check(TokenType::E)) {
        Token op = advance();
        ExprPtr right = parse_not();
        left = make_expr(BinaryExpr{
            binary_op_for(op.type), std::move(left), std::move(right),
            op.line, op.col});
    }
    return left;
}

ExprPtr Parser::parse_not() {
    const NestingGuard guard(*this);
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
    while (!is_error(left) &&
           (check(TokenType::EQUAL)      || check(TokenType::NOT_EQUAL) ||
            check(TokenType::LESS)       || check(TokenType::GREATER)   ||
            check(TokenType::LESS_EQ)    || check(TokenType::GREATER_EQ))) {
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
    while (!is_error(left) && (check(TokenType::PLUS) || check(TokenType::MINUS))) {
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
    while (!is_error(left) && (check(TokenType::STAR) || check(TokenType::SLASH) ||
                               check(TokenType::DIV) || check(TokenType::MOD))) {
        Token op = advance();
        ExprPtr right = parse_unary();
        left = make_expr(BinaryExpr{
            binary_op_for(op.type), std::move(left), std::move(right),
            op.line, op.col});
    }
    return left;
}

ExprPtr Parser::parse_unary() {
    const NestingGuard guard(*this);
    if (check(TokenType::MINUS)) {
        Token op = advance();
        ExprPtr operand = parse_unary();  // recursivo: "--x" funciona
        return make_expr(UnaryExpr{
            UnaryOp::Neg, std::move(operand), op.line, op.col});
    }
    return parse_power();
}

ExprPtr Parser::parse_power() {
    ExprPtr base = parse_primary();
    if (is_error(base) || !check(TokenType::CARET)) return base;
    Token op = advance();
    ExprPtr exponent = parse_unary();
    return make_expr(BinaryExpr{
        BinaryOp::Pow, std::move(base), std::move(exponent), op.line, op.col});
}

ExprPtr Parser::parse_primary() {
    Token tok = peek();  // copia - vamos avancar abaixo

    if (check(TokenType::NUMBER)) {
        advance();
        const bool has_dot = tok.lexeme.find('.') != std::string::npos;
        // O lexer ja validou: stod e seguro.
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
        if (check(TokenType::LPAREN)) return parse_call_expr(tok);
        return parse_name_reference(tok);
    }
    if (check(TokenType::LPAREN)) {
        advance();
        ExprPtr inner = parse_expr();
        if (!match(TokenType::RPAREN)) {
            const SourcePos cur = expected_pos();
            report("E200", "')' esperado",
                         cur.line, cur.col,
                         "feche o parêntese antes de continuar a expressão.");
        }
        return inner;
    }

    // Token inesperado: nao consome. Quem garante progresso e a sincronizacao
    // do comando (parse_stmt_list).
    return missing_expr();
}

} // namespace spike
