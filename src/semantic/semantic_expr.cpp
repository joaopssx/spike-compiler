#include "spike/semantic.hpp"

#include <type_traits>
#include <variant>

namespace spike {

void SemanticAnalyzer::check_expr(const ExprPtr& expr) {
    if (!expr) return;
    std::visit(
        [this](const auto& node) {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, VarExpr>) {
                mark_read(node.name, node.line, node.col);
            } else if constexpr (std::is_same_v<Node, BinaryExpr>) {
                check_expr(node.left);
                check_expr(node.right);
            } else if constexpr (std::is_same_v<Node, UnaryExpr>) {
                check_expr(node.operand);
            }
        },
        expr->node);
}

} // namespace spike
