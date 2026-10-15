#include "semantic/semantic_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/ast.hpp"

#include <cstddef>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

namespace {

using spike_tests::analyze_typed;
using spike_tests::count_occurrences;
using spike_tests::kTypedVars;
using spike_tests::program_with;

std::string assign_errors(const std::string& body) {
    return analyze_typed(program_with(kTypedVars, body))->diag_output;
}

void test_valid_assignments() {
    for (const std::string body : {"  i <- 1\n", "  r <- 1\n", "  r <- 1.5\n", "  r <- i * 2\n",
                                   "  t <- \"x\"\n", "  t <- t + u\n", "  b <- falso\n",
                                   "  b <- i > r\n", "  i := j - 3\n"}) {
        SPIKE_EXPECT(assign_errors(body).empty());
    }
}

void test_real_into_integer_is_e721() {
    for (const std::string body : {"  i <- 1.5\n", "  i <- r\n", "  i <- 7 / 2\n"}) {
        const std::string out = assign_errors(body);
        SPIKE_EXPECT(count_occurrences(out, "erro[") == 1);
        SPIKE_EXPECT(out.find("E721") != std::string::npos);
    }
    const std::string out = assign_errors("  i <- 1.5\n");
    SPIKE_EXPECT(out.find("'i' é inteiro e o valor é real") != std::string::npos);
    SPIKE_EXPECT(out.find("trunca()") != std::string::npos);
}

void test_incompatible_assignments_are_e720() {
    for (const std::string body : {"  i <- \"a\"\n", "  r <- \"a\"\n", "  t <- 1\n",
                                   "  t <- verdadeiro\n", "  b <- 1\n", "  i <- b\n"}) {
        const std::string out = assign_errors(body);
        SPIKE_EXPECT(count_occurrences(out, "erro[") == 1);
        SPIKE_EXPECT(out.find("E720") != std::string::npos);
    }
    const std::string out = assign_errors("  i <- \"a\"\n");
    SPIKE_EXPECT(out.find("não é possível guardar 'caractere' em 'i', que é 'inteiro'") !=
                 std::string::npos);
}

void test_assignment_error_points_at_value() {
    const std::string out = assign_errors("  i <- 2.5 * r\n");
    SPIKE_EXPECT(out.find(":8:8") != std::string::npos);
}

void test_assignment_cascade_is_avoided() {
    const std::string out = assign_errors("  i <- (1 + \"a\") * 2\n");
    SPIKE_EXPECT(count_occurrences(out, "erro[") == 1);
    SPIKE_EXPECT(out.find("E710") != std::string::npos);

    const std::string undeclared = assign_errors("  z <- \"a\"\n  i <- k + 1\n");
    SPIKE_EXPECT(count_occurrences(undeclared, "erro[") == 2);
    SPIKE_EXPECT(count_occurrences(undeclared, "E700") == 2);
}

void collect_exprs(const spike::ExprPtr& expr, std::vector<const spike::Expr*>& out);

void collect_from_node(const spike::Expr& expr, std::vector<const spike::Expr*>& out) {
    std::visit(
        [&out](const auto& node) {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, spike::BinaryExpr>) {
                collect_exprs(node.left, out);
                collect_exprs(node.right, out);
            } else if constexpr (std::is_same_v<Node, spike::UnaryExpr>) {
                collect_exprs(node.operand, out);
            }
        },
        expr.node);
}

void collect_exprs(const spike::ExprPtr& expr, std::vector<const spike::Expr*>& out) {
    if (!expr) return;
    out.push_back(expr.get());
    collect_from_node(*expr, out);
}

void collect_block(const spike::StmtList& stmts, std::vector<const spike::Expr*>& out) {
    for (const auto& stmt : stmts) {
        std::visit(
            [&out](const auto& node) {
                using Node = std::decay_t<decltype(node)>;
                if constexpr (std::is_same_v<Node, spike::AssignStmt>) {
                    collect_exprs(node.value, out);
                } else if constexpr (std::is_same_v<Node, spike::EscrevaStmt>) {
                    for (const auto& arg : node.args) collect_exprs(arg, out);
                } else if constexpr (std::is_same_v<Node, spike::SeStmt>) {
                    collect_exprs(node.condition, out);
                    collect_block(node.then_branch, out);
                    collect_block(node.else_branch, out);
                } else if constexpr (std::is_same_v<Node, spike::EnquantoStmt>) {
                    collect_exprs(node.condition, out);
                    collect_block(node.body, out);
                } else if constexpr (std::is_same_v<Node, spike::ParaStmt>) {
                    collect_exprs(node.from, out);
                    collect_exprs(node.to, out);
                    collect_exprs(node.step, out);
                    collect_block(node.body, out);
                } else if constexpr (std::is_same_v<Node, spike::RepitaStmt>) {
                    collect_block(node.body, out);
                    collect_exprs(node.condition, out);
                }
            },
            stmt->node);
    }
}

void test_every_expression_has_a_type() {
    auto typed = analyze_typed(program_with(
        kTypedVars,
        "  i <- -(j + 2) * 3\n"
        "  r <- i / 2\n"
        "  escreval(\"total: \", r, t + u, nao b)\n"
        "  se (i > 0) e (r <= 1.5) entao\n    t <- \"sim\"\n  senao\n    b <- i <> j\n  fimse\n"
        "  enquanto i < 10 faca\n    i <- i + 1\n  fimenquanto\n"
        "  para j de 1 ate i passo 2 faca\n    escreva(j)\n  fimpara\n"
        "  repita\n    i <- i - 1\n  ate i = 0\n"
        "  i <- t + 1\n"));
    std::vector<const spike::Expr*> all;
    collect_block(typed->program.body, all);
    const auto& types = typed->analyzer->expr_types();
    SPIKE_EXPECT(all.size() > 40);
    SPIKE_EXPECT(types.size() == all.size());
    std::size_t invalid = 0;
    for (const spike::Expr* expr : all) {
        const auto it = types.find(expr);
        SPIKE_EXPECT(it != types.end());
        if (it != types.end() && it->second == spike::TypeKind::Invalid) ++invalid;
    }
    SPIKE_EXPECT(invalid == 1);
}

} // namespace

namespace spike_tests {

void run_semantic_assign_tests() {
    test_valid_assignments();
    test_real_into_integer_is_e721();
    test_incompatible_assignments_are_e720();
    test_assignment_error_points_at_value();
    test_assignment_cascade_is_avoided();
    test_every_expression_has_a_type();
}

} // namespace spike_tests
