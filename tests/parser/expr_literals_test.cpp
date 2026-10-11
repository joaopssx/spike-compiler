#include "parser/parser_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/ast.hpp"

#include <string>
#include <variant>

namespace {

using spike_tests::parse_expression;

// ---- parser: expressions (PROMPT 015) -------------------------------------

void test_expr_number_literal() {
    auto r = parse_expression("42");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.expr != nullptr);
    SPIKE_EXPECT(std::holds_alternative<spike::NumberExpr>(r.expr->node));
    const auto& n = std::get<spike::NumberExpr>(r.expr->node);
    SPIKE_EXPECT(n.value == 42.0);
    SPIKE_EXPECT(n.is_integer == true);
}

void test_expr_real_literal() {
    auto r = parse_expression("3.14");
    SPIKE_EXPECT(r.had_error == false);
    const auto& n = std::get<spike::NumberExpr>(r.expr->node);
    SPIKE_EXPECT(n.value > 3.13 && n.value < 3.15);
    SPIKE_EXPECT(n.is_integer == false);
}

void test_expr_var() {
    auto r = parse_expression("contador");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(std::holds_alternative<spike::VarExpr>(r.expr->node));
    SPIKE_EXPECT(std::get<spike::VarExpr>(r.expr->node).name == "contador");
}

void test_expr_bool_literals() {
    {
        auto r = parse_expression("verdadeiro");
        SPIKE_EXPECT(r.had_error == false);
        SPIKE_EXPECT(std::get<spike::BoolExpr>(r.expr->node).value == true);
    }
    {
        auto r = parse_expression("falso");
        SPIKE_EXPECT(r.had_error == false);
        SPIKE_EXPECT(std::get<spike::BoolExpr>(r.expr->node).value == false);
    }
}

} // namespace

namespace spike_tests {

void run_parser_expr_literal_tests() {
    test_expr_number_literal();
    test_expr_real_literal();
    test_expr_var();
    test_expr_bool_literals();
}

} // namespace spike_tests
