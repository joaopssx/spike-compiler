#include "codegen/codegen_internal.hpp"

#include <string>
#include <type_traits>
#include <variant>

namespace spike::codegen_detail {

namespace {

const char* c_operator(BinaryOp op) {
    switch (op) {
        case BinaryOp::Add: return "+";
        case BinaryOp::Sub: return "-";
        case BinaryOp::Mul: return "*";
        case BinaryOp::Div: return "/";
        case BinaryOp::Eq:  return "==";
        case BinaryOp::Ne:  return "!=";
        case BinaryOp::Lt:  return "<";
        case BinaryOp::Gt:  return ">";
        case BinaryOp::Le:  return "<=";
        case BinaryOp::Ge:  return ">=";
        case BinaryOp::And: return "&&";
        case BinaryOp::Or:  return "||";
    }
    return "?";
}

const char* wrapping_int_function(BinaryOp op) {
    switch (op) {
        case BinaryOp::Add: return "spike_add_int";
        case BinaryOp::Sub: return "spike_sub_int";
        case BinaryOp::Mul: return "spike_mul_int";
        default:            return nullptr;
    }
}

bool is_comparison(BinaryOp op) {
    return op == BinaryOp::Eq || op == BinaryOp::Ne || op == BinaryOp::Lt ||
           op == BinaryOp::Gt || op == BinaryOp::Le || op == BinaryOp::Ge;
}

SourcePos node_pos(const Expr& expr) {
    return std::visit([](const auto& node) { return SourcePos{node.line, node.col}; },
                      expr.node);
}

} // namespace

TypeKind Generator::type_of(const Expr& expr) {
    const auto it = types_.find(&expr);
    const TypeKind type = it == types_.end() ? TypeKind::None : it->second;
    if (type == TypeKind::None || type == TypeKind::Invalid) {
        const SourcePos pos = node_pos(expr);
        fail("E900", "erro interno do gerador: expressão sem tipo válido", pos.line, pos.col);
    }
    return type;
}

std::string Generator::expr_as(const ExprPtr& value, TypeKind wanted) {
    if (!value) {
        fail("E900", "erro interno do gerador: expressão vazia", 0, 0);
        return "";
    }
    const std::string text = expr(value);
    if (wanted == TypeKind::Real && type_of(*value) == TypeKind::Integer) {
        return "((spike_real)" + text + ")";
    }
    return text;
}

std::string Generator::expr(const ExprPtr& value) {
    if (!value) {
        fail("E900", "erro interno do gerador: expressão vazia", 0, 0);
        return "";
    }
    const TypeKind type = type_of(*value);
    if (failed()) return "";
    return std::visit(
        [this, type](const auto& node) -> std::string {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, NumberExpr>) {
                return node.is_integer ? int_literal(node.value) : real_literal(node.value);
            } else if constexpr (std::is_same_v<Node, StringExpr>) {
                return string_literal(node.value);
            } else if constexpr (std::is_same_v<Node, BoolExpr>) {
                return node.value ? "1" : "0";
            } else if constexpr (std::is_same_v<Node, VarExpr>) {
                return c_name(node.name);
            } else if constexpr (std::is_same_v<Node, BinaryExpr>) {
                return binary(node, type);
            } else {
                return unary(node);
            }
        },
        value->node);
}

std::string Generator::binary(const BinaryExpr& node, TypeKind result) {
    if (!node.left || !node.right) {
        fail("E900", "erro interno do gerador: operação incompleta", node.line, node.col);
        return "";
    }
    const TypeKind left_type = type_of(*node.left);
    const TypeKind right_type = type_of(*node.right);
    if (failed()) return "";
    const bool text = left_type == TypeKind::String && right_type == TypeKind::String;
    const bool to_real = left_type != right_type || node.op == BinaryOp::Div;
    const TypeKind operand = to_real ? TypeKind::Real : left_type;
    const std::string a = expr_as(node.left, operand);
    const std::string b = expr_as(node.right, operand);

    if (node.op == BinaryOp::Add && text) return "spike_str_concat(" + a + ", " + b + ")";
    if (node.op == BinaryOp::Div) {
        return "spike_div_real(" + a + ", " + b + ", " + std::to_string(node.line) + ")";
    }
    if (result == TypeKind::Integer) {
        if (const char* function = wrapping_int_function(node.op)) {
            return std::string(function) + "(" + a + ", " + b + ")";
        }
    }
    if (text && is_comparison(node.op)) {
        if (node.op == BinaryOp::Eq) return "spike_str_eq(" + a + ", " + b + ")";
        if (node.op == BinaryOp::Ne) return "(!spike_str_eq(" + a + ", " + b + "))";
        return "(spike_str_cmp(" + a + ", " + b + ") " + c_operator(node.op) + " 0)";
    }
    return "(" + a + " " + c_operator(node.op) + " " + b + ")";
}

std::string Generator::unary(const UnaryExpr& node) {
    if (!node.operand) {
        fail("E900", "erro interno do gerador: operação incompleta", node.line, node.col);
        return "";
    }
    const TypeKind operand_type = type_of(*node.operand);
    const std::string value = expr(node.operand);
    if (node.op == UnaryOp::Not) return "(!" + value + ")";
    if (std::holds_alternative<NumberExpr>(node.operand->node)) return "(-" + value + ")";
    if (operand_type == TypeKind::Integer) return "spike_sub_int(0LL, " + value + ")";
    return "(-" + value + ")";
}

} // namespace spike::codegen_detail
