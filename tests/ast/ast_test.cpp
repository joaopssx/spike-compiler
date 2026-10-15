#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/ast.hpp"

#include <utility>
#include <variant>

namespace {

// ---- ast ------------------------------------------------------------------

void test_ast_leaf_exprs_carry_position() {
    auto n = spike::make_expr(spike::NumberExpr{42.0, true, 3, 5});
    SPIKE_EXPECT(std::holds_alternative<spike::NumberExpr>(n->node));
    const auto& num = std::get<spike::NumberExpr>(n->node);
    SPIKE_EXPECT(num.value == 42.0);
    SPIKE_EXPECT(num.is_integer == true);
    SPIKE_EXPECT(num.line == 3);
    SPIKE_EXPECT(num.col == 5);

    auto s = spike::make_expr(spike::StringExpr{"oi", 1, 1});
    SPIKE_EXPECT(std::get<spike::StringExpr>(s->node).value == "oi");

    auto b = spike::make_expr(spike::BoolExpr{true, 1, 1});
    SPIKE_EXPECT(std::get<spike::BoolExpr>(b->node).value);
}

void test_ast_binary_expr_is_recursive() {
    // Constroi a árvore para "x + 1"
    auto left  = spike::make_expr(spike::VarExpr{"x", 1, 1});
    auto right = spike::make_expr(spike::NumberExpr{1.0, true, 1, 5});
    auto sum   = spike::make_expr(
        spike::BinaryExpr{spike::BinaryOp::Add, std::move(left), std::move(right), 1, 3});

    SPIKE_EXPECT(std::holds_alternative<spike::BinaryExpr>(sum->node));
    const auto& bin = std::get<spike::BinaryExpr>(sum->node);
    SPIKE_EXPECT(bin.op == spike::BinaryOp::Add);
    SPIKE_EXPECT(std::holds_alternative<spike::VarExpr>(bin.left->node));
    SPIKE_EXPECT(std::holds_alternative<spike::NumberExpr>(bin.right->node));
}

void test_ast_program_assembles() {
    spike::Program prog;
    prog.name = "calc";
    prog.globals.push_back(spike::VarDecl{"x", spike::TypeKind::Integer, nullptr, 2, 4});

    // x <- 42
    auto val = spike::make_expr(spike::NumberExpr{42.0, true, 4, 9});
    prog.body.push_back(spike::make_stmt(
        spike::AssignStmt{"x", std::move(val), 4, 4}));

    // se x > 0 entao escreval("positivo") fimse
    auto cond = spike::make_expr(spike::BinaryExpr{
        spike::BinaryOp::Gt,
        spike::make_expr(spike::VarExpr{"x", 5, 7}),
        spike::make_expr(spike::NumberExpr{0.0, true, 5, 11}),
        5, 9});

    spike::StmtList then_block;
    spike::EscrevaStmt esc;
    esc.newline = true;
    esc.line = 6; esc.col = 7;
    esc.args.push_back(spike::make_expr(spike::StringExpr{"positivo", 6, 16}));
    then_block.push_back(spike::make_stmt(std::move(esc)));

    prog.body.push_back(spike::make_stmt(spike::SeStmt{
        std::move(cond), std::move(then_block), {}, 5, 4}));

    // Verifica que ficou tudo no lugar.
    SPIKE_EXPECT(prog.name == "calc");
    SPIKE_EXPECT(prog.globals.size() == 1);
    SPIKE_EXPECT(prog.globals[0].name == "x");
    SPIKE_EXPECT(prog.body.size() == 2);
    SPIKE_EXPECT(std::holds_alternative<spike::AssignStmt>(prog.body[0]->node));
    SPIKE_EXPECT(std::holds_alternative<spike::SeStmt>(prog.body[1]->node));

    const auto& se = std::get<spike::SeStmt>(prog.body[1]->node);
    SPIKE_EXPECT(se.then_branch.size() == 1);
    SPIKE_EXPECT(se.else_branch.empty());
}

} // namespace

namespace spike_tests {

void run_ast_tests() {
    test_ast_leaf_exprs_carry_position();
    test_ast_binary_expr_is_recursive();
    test_ast_program_assembles();
}

} // namespace spike_tests
