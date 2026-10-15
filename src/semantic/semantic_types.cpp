#include "spike/semantic.hpp"
#include "semantic/semantic_internal.hpp"

#include <string>

namespace spike {

using semantic_detail::expr_start;
using semantic_detail::is_numeric;
using semantic_detail::quoted;

namespace {

bool is_arithmetic(BinaryOp op) {
    return op == BinaryOp::Add || op == BinaryOp::Sub ||
           op == BinaryOp::Mul || op == BinaryOp::Div;
}

bool is_logical(BinaryOp op) {
    return op == BinaryOp::And || op == BinaryOp::Or;
}

bool is_equality(BinaryOp op) {
    return op == BinaryOp::Eq || op == BinaryOp::Ne;
}

std::string op_text(BinaryOp op) {
    return std::string("'") + to_string(op) + "'";
}

TypeKind arithmetic_result(BinaryOp op, TypeKind left, TypeKind right) {
    if (op == BinaryOp::Add && left == TypeKind::String && right == TypeKind::String) {
        return TypeKind::String;
    }
    if (!is_numeric(left) || !is_numeric(right)) return TypeKind::Invalid;
    if (op == BinaryOp::Div) return TypeKind::Real;
    if (left == TypeKind::Integer && right == TypeKind::Integer) return TypeKind::Integer;
    return TypeKind::Real;
}

bool comparable(BinaryOp op, TypeKind left, TypeKind right) {
    if (is_numeric(left) && is_numeric(right)) return true;
    if (left == TypeKind::String && right == TypeKind::String) return true;
    return is_equality(op) && left == TypeKind::Bool && right == TypeKind::Bool;
}

std::string arithmetic_hint(BinaryOp op, TypeKind left, TypeKind right) {
    const bool text_and_number = (left == TypeKind::String && is_numeric(right)) ||
                                 (is_numeric(left) && right == TypeKind::String);
    if (text_and_number) {
        return "para juntar texto e número, converta o número com paraTexto().";
    }
    if (op == BinaryOp::Add) return "'+' soma dois números ou junta dois textos.";
    return "o operador " + op_text(op) + " só funciona com números.";
}

} // namespace

TypeKind SemanticAnalyzer::check_binary(const BinaryExpr& node, TypeKind left, TypeKind right) {
    if (left == TypeKind::Invalid || right == TypeKind::Invalid) return TypeKind::Invalid;

    if (is_arithmetic(node.op)) {
        const TypeKind result = arithmetic_result(node.op, left, right);
        if (result == TypeKind::Invalid) {
            diags_.error("E710",
                         "o operador " + op_text(node.op) + " não funciona entre " +
                             quoted(left) + " e " + quoted(right),
                         node.line, node.col, arithmetic_hint(node.op, left, right));
        }
        return result;
    }

    if (is_logical(node.op)) {
        if (left == TypeKind::Bool && right == TypeKind::Bool) return TypeKind::Bool;
        const TypeKind wrong = left != TypeKind::Bool ? left : right;
        diags_.error("E712",
                     "o operador " + op_text(node.op) +
                         " precisa de dois valores do tipo logico, mas recebeu " + quoted(wrong),
                     node.line, node.col,
                     "crie valores do tipo logico com comparações, como 'x > 0'.");
        return TypeKind::Invalid;
    }

    if (comparable(node.op, left, right)) return TypeKind::Bool;
    const bool both_bool = left == TypeKind::Bool && right == TypeKind::Bool;
    diags_.error("E711",
                 "não é possível comparar " + quoted(left) + " com " + quoted(right) +
                     " usando " + op_text(node.op),
                 node.line, node.col,
                 both_bool ? "valores do tipo logico só podem ser comparados com '=' e '<>'."
                           : "compare valores do mesmo tipo: números com números, "
                             "textos com textos.");
    return TypeKind::Invalid;
}

TypeKind SemanticAnalyzer::check_unary(const UnaryExpr& node, TypeKind operand) {
    if (operand == TypeKind::Invalid) return TypeKind::Invalid;
    if (node.op == UnaryOp::Not) {
        if (operand == TypeKind::Bool) return TypeKind::Bool;
        diags_.error("E713",
                     "o operador 'nao' precisa de um valor do tipo logico, mas recebeu " +
                         quoted(operand),
                     node.line, node.col,
                     "'nao' inverte verdadeiro e falso; para números, use uma comparação.");
        return TypeKind::Invalid;
    }
    if (is_numeric(operand)) return operand;
    diags_.error("E713",
                 "o operador '-' precisa de um número, mas recebeu " + quoted(operand),
                 node.line, node.col, "o sinal de menos só funciona com inteiro e real.");
    return TypeKind::Invalid;
}

void SemanticAnalyzer::check_assign(const AssignStmt& stmt) {
    const TypeKind value = check_expr(stmt.value);
    Symbol* target = resolve(stmt.var_name, stmt.line, stmt.col);
    if (!target) return;
    target->assigned = true;

    const TypeKind declared = target->type;
    if (value == TypeKind::Invalid || declared == TypeKind::Invalid ||
        declared == TypeKind::None || !stmt.value) {
        return;
    }
    if (value == declared) return;
    if (declared == TypeKind::Real && value == TypeKind::Integer) return;

    const SourcePos pos = expr_start(*stmt.value);
    if (declared == TypeKind::Integer && value == TypeKind::Real) {
        diags_.error("E721", "'" + stmt.var_name + "' é inteiro e o valor é real",
                     pos.line, pos.col,
                     "use trunca() ou arredonda() para transformar o valor em inteiro.");
        return;
    }
    diags_.error("E720",
                 "não é possível guardar " + quoted(value) + " em '" + stmt.var_name +
                     "', que é " + quoted(declared),
                 pos.line, pos.col, "o valor precisa ser do mesmo tipo da variável.");
}

} // namespace spike
