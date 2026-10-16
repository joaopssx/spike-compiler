#include "spike/semantic.hpp"
#include "semantic/semantic_internal.hpp"

#include <string>
#include <variant>

namespace spike {

using semantic_detail::expr_start;
using semantic_detail::quoted;

namespace {

bool is_literal_zero(const Expr& expr) {
    if (const auto* number = std::get_if<NumberExpr>(&expr.node)) {
        return number->value == 0.0;
    }
    if (const auto* unary = std::get_if<UnaryExpr>(&expr.node)) {
        return unary->op == UnaryOp::Neg && unary->operand && is_literal_zero(*unary->operand);
    }
    return false;
}

std::string condition_hint(const Expr& condition, TypeKind type) {
    const auto* var = std::get_if<VarExpr>(&condition.node);
    if (var && type == TypeKind::Integer) {
        return "para testar o número, compare: '" + var->name + " <> 0' ou '" + var->name +
               " = 1'.";
    }
    return "compare com '=', '<', '>' etc.";
}

} // namespace

void SemanticAnalyzer::check_condition(const ExprPtr& condition, const char* command) {
    const TypeKind type = check_expr(condition);
    if (!condition || type == TypeKind::Invalid || type == TypeKind::Bool) return;
    const SourcePos pos = expr_start(*condition);
    diags_.error("E730",
                 std::string("a condição do '") + command + "' precisa ser logica, mas é " +
                     quoted(type),
                 pos.line, pos.col, condition_hint(*condition, type));
}

void SemanticAnalyzer::check_para_bound(const ExprPtr& bound, const char* part) {
    const TypeKind type = check_expr(bound);
    if (!bound || type == TypeKind::Invalid || type == TypeKind::Integer) return;
    const SourcePos pos = expr_start(*bound);
    diags_.error("E732",
                 std::string("o ") + part + " do 'para' precisa ser 'inteiro', mas é " +
                     quoted(type),
                 pos.line, pos.col,
                 type == TypeKind::Real
                     ? "o 'para' conta de um em um inteiro; use valores inteiros ou trunca()."
                     : "o 'para' conta com números inteiros.");
}

void SemanticAnalyzer::check_para(const ParaStmt& stmt) {
    const int line = stmt.var_pos.line != 0 ? stmt.var_pos.line : stmt.line;
    const int col = stmt.var_pos.line != 0 ? stmt.var_pos.col : stmt.col;
    const Symbol* counter = mark_assigned(stmt.var_name, line, col);
    if (counter && !counter->dims.empty()) {
        diags_.error("E731", "a variável do 'para' não pode ser um vetor", line, col,
                     "use uma variável inteira simples para contar as voltas.");
    } else if (counter && counter->type != TypeKind::Integer && counter->type != TypeKind::Invalid) {
        diags_.error("E731",
                     "a variável do 'para' precisa ser 'inteiro', mas '" + stmt.var_name +
                         "' é " + quoted(counter->type),
                     line, col,
                     "declare '" + counter->name + "' como inteiro ou use outra variável.");
    }

    check_para_bound(stmt.from, "início");
    check_para_bound(stmt.to, "fim");
    check_para_bound(stmt.step, "passo");
    if (stmt.step && expr_types_[stmt.step.get()] == TypeKind::Integer &&
        is_literal_zero(*stmt.step)) {
        const SourcePos pos = expr_start(*stmt.step);
        diags_.error("E733", "'passo 0' faz o 'para' repetir para sempre", pos.line, pos.col,
                     "use um passo diferente de zero, como 'passo 1' ou 'passo -1'.");
    }

    active_loop_vars_.push_back(symbol_key(stmt.var_name));
    check_loop_body(stmt.body);
    active_loop_vars_.pop_back();
}

} // namespace spike
