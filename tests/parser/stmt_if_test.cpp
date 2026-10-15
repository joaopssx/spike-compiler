#include "parser/parser_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/ast.hpp"

#include <string>
#include <variant>

namespace {

using spike_tests::parse_source;

// ---- parser: se statement -------------------------------------------------

void test_stmt_se_then_only() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   se x > 0 entao\n"
        "      escreva(\"pos\")\n"
        "   fimse\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.body.size() == 1);
    SPIKE_EXPECT(std::holds_alternative<spike::SeStmt>(r.program.body[0]->node));
    const auto& s = std::get<spike::SeStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(std::holds_alternative<spike::BinaryExpr>(s.condition->node));
    SPIKE_EXPECT(std::get<spike::BinaryExpr>(s.condition->node).op == spike::BinaryOp::Gt);
    SPIKE_EXPECT(s.then_branch.size() == 1);
    SPIKE_EXPECT(s.else_branch.empty());
    SPIKE_EXPECT(std::holds_alternative<spike::EscrevaStmt>(s.then_branch[0]->node));
}

void test_stmt_se_with_senao() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   se x > 0 entao\n"
        "      escreva(\"pos\")\n"
        "   senao\n"
        "      escreva(\"neg\")\n"
        "   fimse\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    const auto& s = std::get<spike::SeStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(s.then_branch.size() == 1);
    SPIKE_EXPECT(s.else_branch.size() == 1);
    SPIKE_EXPECT(std::holds_alternative<spike::EscrevaStmt>(s.then_branch[0]->node));
    SPIKE_EXPECT(std::holds_alternative<spike::EscrevaStmt>(s.else_branch[0]->node));
}

void test_stmt_se_nested() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   se a entao\n"
        "      se b entao\n"
        "         escreva(\"nested\")\n"
        "      fimse\n"
        "   fimse\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    const auto& outer = std::get<spike::SeStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(outer.then_branch.size() == 1);
    SPIKE_EXPECT(std::holds_alternative<spike::SeStmt>(outer.then_branch[0]->node));
    const auto& inner = std::get<spike::SeStmt>(outer.then_branch[0]->node);
    SPIKE_EXPECT(inner.then_branch.size() == 1);
    SPIKE_EXPECT(std::holds_alternative<spike::EscrevaStmt>(inner.then_branch[0]->node));
}

void test_stmt_E400_missing_entao() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   se x > 0\n"
        "      escreva(\"oi\")\n"
        "   fimse\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E400") != std::string::npos);
}

void test_stmt_E401_missing_fimse() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   se x entao\n"
        "      escreva(\"oi\")\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E401") != std::string::npos);
}

} // namespace

namespace spike_tests {

void run_parser_stmt_if_tests() {
    test_stmt_se_then_only();
    test_stmt_se_with_senao();
    test_stmt_se_nested();
    test_stmt_E400_missing_entao();
    test_stmt_E401_missing_fimse();
}

} // namespace spike_tests
