#include "parser/parser_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/ast.hpp"

#include <string>
#include <variant>

namespace {

using spike_tests::parse_expression;

// ---- parser: expressions (PROMPT 015) -------------------------------------

void test_expr_addition() {
    auto r = parse_expression("x + 1");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(std::holds_alternative<spike::BinaryExpr>(r.expr->node));
    const auto& b = std::get<spike::BinaryExpr>(r.expr->node);
    SPIKE_EXPECT(b.op == spike::BinaryOp::Add);
    SPIKE_EXPECT(std::holds_alternative<spike::VarExpr>(b.left->node));
    SPIKE_EXPECT(std::get<spike::VarExpr>(b.left->node).name == "x");
    SPIKE_EXPECT(std::holds_alternative<spike::NumberExpr>(b.right->node));
    SPIKE_EXPECT(std::get<spike::NumberExpr>(b.right->node).value == 1.0);
}

void test_expr_unary_not() {
    auto r = parse_expression("nao verdadeiro");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(std::holds_alternative<spike::UnaryExpr>(r.expr->node));
    const auto& u = std::get<spike::UnaryExpr>(r.expr->node);
    SPIKE_EXPECT(u.op == spike::UnaryOp::Not);
    SPIKE_EXPECT(std::holds_alternative<spike::BoolExpr>(u.operand->node));
    SPIKE_EXPECT(std::get<spike::BoolExpr>(u.operand->node).value == true);
}

void test_expr_paren_changes_precedence() {
    // (3 + 4) * 2 → raiz é o '*', filho esquerdo é o '+'.
    auto r = parse_expression("(3 + 4) * 2");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(std::holds_alternative<spike::BinaryExpr>(r.expr->node));
    const auto& mul = std::get<spike::BinaryExpr>(r.expr->node);
    SPIKE_EXPECT(mul.op == spike::BinaryOp::Mul);
    SPIKE_EXPECT(std::holds_alternative<spike::BinaryExpr>(mul.left->node));
    SPIKE_EXPECT(std::get<spike::BinaryExpr>(mul.left->node).op == spike::BinaryOp::Add);
    SPIKE_EXPECT(std::holds_alternative<spike::NumberExpr>(mul.right->node));
    SPIKE_EXPECT(std::get<spike::NumberExpr>(mul.right->node).value == 2.0);
}

void test_expr_precedence_without_parens() {
    // 1 + 2 * 3 → 1 + (2 * 3); raiz é '+'.
    auto r = parse_expression("1 + 2 * 3");
    SPIKE_EXPECT(r.had_error == false);
    const auto& add = std::get<spike::BinaryExpr>(r.expr->node);
    SPIKE_EXPECT(add.op == spike::BinaryOp::Add);
    SPIKE_EXPECT(std::holds_alternative<spike::NumberExpr>(add.left->node));
    SPIKE_EXPECT(std::get<spike::NumberExpr>(add.left->node).value == 1.0);
    const auto& mul = std::get<spike::BinaryExpr>(add.right->node);
    SPIKE_EXPECT(mul.op == spike::BinaryOp::Mul);
}

void test_expr_comparison_lower_than_arithmetic() {
    // a + 1 < b * 2 → comparação na raiz.
    auto r = parse_expression("a + 1 < b * 2");
    SPIKE_EXPECT(r.had_error == false);
    const auto& cmp = std::get<spike::BinaryExpr>(r.expr->node);
    SPIKE_EXPECT(cmp.op == spike::BinaryOp::Lt);
}

void test_expr_and_lower_than_comparison() {
    // a > 1 e b < 2 → 'e' na raiz, comparações nos filhos.
    auto r = parse_expression("a > 1 e b < 2");
    SPIKE_EXPECT(r.had_error == false);
    const auto& and_node = std::get<spike::BinaryExpr>(r.expr->node);
    SPIKE_EXPECT(and_node.op == spike::BinaryOp::And);
    SPIKE_EXPECT(std::get<spike::BinaryExpr>(and_node.left->node).op == spike::BinaryOp::Gt);
    SPIKE_EXPECT(std::get<spike::BinaryExpr>(and_node.right->node).op == spike::BinaryOp::Lt);
}

void test_expr_or_keyword_case_insensitive() {
    // Lexer normaliza keywords no match; binary_op_for converte o token no enum.
    auto r = parse_expression("verdadeiro OU falso");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(std::get<spike::BinaryExpr>(r.expr->node).op == spike::BinaryOp::Or);
}

void test_expr_unary_minus() {
    auto r = parse_expression("-x");
    SPIKE_EXPECT(r.had_error == false);
    const auto& u = std::get<spike::UnaryExpr>(r.expr->node);
    SPIKE_EXPECT(u.op == spike::UnaryOp::Neg);
    SPIKE_EXPECT(std::get<spike::VarExpr>(u.operand->node).name == "x");
}

void test_expr_unclosed_paren_emits_E200() {
    spike::Lexer lex("(1 + 2", "<test>");
    auto toks = lex.tokenize();
    spike::Parser p(std::move(toks), "<test>", "(1 + 2");
    p.parse_expr();
    SPIKE_EXPECT(p.had_error());
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    p.diagnostics().print_all();
    std::cerr.rdbuf(old);
    SPIKE_EXPECT(sink.str().find("E200") != std::string::npos);
}

void test_expr_unexpected_token_emits_E201() {
    spike::Lexer lex("+", "<test>");
    auto toks = lex.tokenize();
    spike::Parser p(std::move(toks), "<test>", "+");
    p.parse_expr();
    SPIKE_EXPECT(p.had_error());
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    p.diagnostics().print_all();
    std::cerr.rdbuf(old);
    SPIKE_EXPECT(sink.str().find("E201") != std::string::npos);
}

} // namespace

namespace spike_tests {

void run_parser_expr_operator_tests() {
    test_expr_addition();
    test_expr_unary_not();
    test_expr_paren_changes_precedence();
    test_expr_precedence_without_parens();
    test_expr_comparison_lower_than_arithmetic();
    test_expr_and_lower_than_comparison();
    test_expr_or_keyword_case_insensitive();
    test_expr_unary_minus();
    test_expr_unclosed_paren_emits_E200();
    test_expr_unexpected_token_emits_E201();
}

} // namespace spike_tests
