#include "parser/parser_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/ast.hpp"
#include "spike/token.hpp"

#include <string>
#include <variant>
#include <vector>

namespace {

using spike::BinaryExpr;
using spike::BinaryOp;
using spike::TokenType;
using spike_tests::parse_expression;
using spike_tests::parse_source;
using spike_tests::tokens_of;

const BinaryExpr* as_binary(const spike::ExprPtr& expr) {
    return expr ? std::get_if<BinaryExpr>(&expr->node) : nullptr;
}

bool is_number(const spike::ExprPtr& expr, double value) {
    const auto* number = expr ? std::get_if<spike::NumberExpr>(&expr->node) : nullptr;
    return number && number->value == value;
}

bool is_var(const spike::ExprPtr& expr, const std::string& name) {
    const auto* var = expr ? std::get_if<spike::VarExpr>(&expr->node) : nullptr;
    return var && var->name == name;
}

void test_lexer_div_mod_caret() {
    const auto toks = tokens_of("div MOD Div dividir modo ^");
    SPIKE_EXPECT(toks.size() == 6);
    if (toks.size() != 6) return;
    SPIKE_EXPECT(toks[0].type == TokenType::DIV);
    SPIKE_EXPECT(toks[1].type == TokenType::MOD);
    SPIKE_EXPECT(toks[2].type == TokenType::DIV);
    SPIKE_EXPECT(toks[3].type == TokenType::IDENTIFIER);
    SPIKE_EXPECT(toks[4].type == TokenType::IDENTIFIER);
    SPIKE_EXPECT(toks[5].type == TokenType::CARET);
    SPIKE_EXPECT(spike::token_type_to_string(TokenType::CARET) == std::string("CARET"));
}

void test_div_and_mod_share_level_with_star() {
    auto r = parse_expression("a * b div c mod d");
    SPIKE_EXPECT(!r.had_error);
    const auto* mod = as_binary(r.expr);
    SPIKE_EXPECT(mod && mod->op == BinaryOp::Mod && is_var(mod->right, "d"));
    const auto* div = mod ? as_binary(mod->left) : nullptr;
    SPIKE_EXPECT(div && div->op == BinaryOp::IntDiv && is_var(div->right, "c"));
    const auto* mul = div ? as_binary(div->left) : nullptr;
    SPIKE_EXPECT(mul && mul->op == BinaryOp::Mul);
}

void test_div_binds_tighter_than_plus() {
    auto r = parse_expression("a + b div c");
    const auto* add = as_binary(r.expr);
    SPIKE_EXPECT(add && add->op == BinaryOp::Add && is_var(add->left, "a"));
    const auto* div = add ? as_binary(add->right) : nullptr;
    SPIKE_EXPECT(div && div->op == BinaryOp::IntDiv);
}

void test_power_binds_tighter_than_unary_minus() {
    auto r = parse_expression("-2 ^ 2");
    const auto* neg = r.expr ? std::get_if<spike::UnaryExpr>(&r.expr->node) : nullptr;
    SPIKE_EXPECT(neg && neg->op == spike::UnaryOp::Neg);
    const auto* pow = neg ? as_binary(neg->operand) : nullptr;
    SPIKE_EXPECT(pow && pow->op == BinaryOp::Pow && is_number(pow->left, 2) &&
                 is_number(pow->right, 2));
}

void test_power_is_right_associative() {
    auto r = parse_expression("2 ^ 3 ^ 2");
    const auto* outer = as_binary(r.expr);
    SPIKE_EXPECT(outer && outer->op == BinaryOp::Pow && is_number(outer->left, 2));
    const auto* inner = outer ? as_binary(outer->right) : nullptr;
    SPIKE_EXPECT(inner && inner->op == BinaryOp::Pow && is_number(inner->left, 3) &&
                 is_number(inner->right, 2));
}

void test_power_binds_tighter_than_star() {
    auto r = parse_expression("2 * 3 ^ 2");
    const auto* mul = as_binary(r.expr);
    SPIKE_EXPECT(mul && mul->op == BinaryOp::Mul);
    const auto* pow = mul ? as_binary(mul->right) : nullptr;
    SPIKE_EXPECT(pow && pow->op == BinaryOp::Pow);
}

void test_power_accepts_negative_exponent() {
    auto r = parse_expression("2 ^ -1");
    SPIKE_EXPECT(!r.had_error);
    const auto* pow = as_binary(r.expr);
    SPIKE_EXPECT(pow && pow->op == BinaryOp::Pow &&
                 std::holds_alternative<spike::UnaryExpr>(pow->right->node));
}

void test_power_without_operand_is_e201() {
    auto r = parse_source("algoritmo \"t\"\nvar\n  x : real\ninicio\n  x <- 2 ^\nfimalgoritmo\n");
    SPIKE_EXPECT(r.had_error);
    SPIKE_EXPECT(r.diag_output.find("E201") != std::string::npos);
}

} // namespace

namespace spike_tests {

void run_parser_expr_power_tests() {
    test_lexer_div_mod_caret();
    test_div_and_mod_share_level_with_star();
    test_div_binds_tighter_than_plus();
    test_power_binds_tighter_than_unary_minus();
    test_power_is_right_associative();
    test_power_binds_tighter_than_star();
    test_power_accepts_negative_exponent();
    test_power_without_operand_is_e201();
}

} // namespace spike_tests
