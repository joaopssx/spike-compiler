#include "spike/semantic.hpp"
#include "semantic/semantic_internal.hpp"

#include <type_traits>
#include <variant>

namespace spike {

namespace semantic_detail {

bool is_numeric(TypeKind type) {
    return type == TypeKind::Integer || type == TypeKind::Real;
}

std::string quoted(TypeKind type) {
    return std::string("'") + type_name(type) + "'";
}

SourcePos expr_start(const Expr& expr) {
    return std::visit(
        [](const auto& node) -> SourcePos {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, BinaryExpr>) {
                if (node.left) return expr_start(*node.left);
            }
            return SourcePos{node.line, node.col};
        },
        expr.node);
}

} // namespace semantic_detail

TypeKind SemanticAnalyzer::check_expr(const ExprPtr& expr) {
    if (!expr) return TypeKind::Invalid;
    const TypeKind type = std::visit(
        [this](const auto& node) -> TypeKind {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, NumberExpr>) {
                return node.is_integer ? TypeKind::Integer : TypeKind::Real;
            } else if constexpr (std::is_same_v<Node, StringExpr>) {
                return TypeKind::String;
            } else if constexpr (std::is_same_v<Node, BoolExpr>) {
                return TypeKind::Bool;
            } else if constexpr (std::is_same_v<Node, VarExpr>) {
                Symbol* symbol = resolve(node.name, node.line, node.col);
                if (!symbol) return TypeKind::Invalid;
                symbol->used = true;
                return symbol->type;
            } else if constexpr (std::is_same_v<Node, BinaryExpr>) {
                const TypeKind left = check_expr(node.left);
                const TypeKind right = check_expr(node.right);
                return check_binary(node, left, right);
            } else {
                return check_unary(node, check_expr(node.operand));
            }
        },
        expr->node);
    expr_types_[expr.get()] = type;
    return type;
}

} // namespace spike
