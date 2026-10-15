#include "codegen/codegen_internal.hpp"

#include <string>
#include <variant>

namespace spike::codegen_detail {

namespace {

bool constant_int(const ExprPtr& value, double& out) {
    if (!value) return false;
    if (const auto* number = std::get_if<NumberExpr>(&value->node)) {
        out = number->value;
        return number->is_integer;
    }
    if (const auto* unary = std::get_if<UnaryExpr>(&value->node)) {
        if (unary->op != UnaryOp::Neg || !constant_int(unary->operand, out)) return false;
        out = -out;
        return true;
    }
    return false;
}

bool outer_parens_wrap_everything(const std::string& text) {
    if (text.size() < 2 || text.front() != '(' || text.back() != ')') return false;
    int depth = 0;
    bool in_string = false;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        if (in_string) {
            if (c == '\\') ++i;
            else if (c == '"') in_string = false;
            continue;
        }
        if (c == '"') in_string = true;
        else if (c == '(') ++depth;
        else if (c == ')' && --depth == 0 && i + 1 < text.size()) return false;
    }
    return true;
}

} // namespace

std::string Generator::condition(const ExprPtr& value) {
    std::string text = expr(value);
    if (outer_parens_wrap_everything(text)) text = text.substr(1, text.size() - 2);
    return text;
}

int Generator::next_loop_id() {
    return ++loop_count_;
}

void Generator::emit_body(const StmtList& stmts) {
    out_.indent();
    emit_block(stmts);
    out_.dedent();
}

void Generator::emit_se(const SeStmt& stmt) {
    const std::string test = condition(stmt.condition);
    if (failed()) return;
    out_.line("if (" + test + ") {");
    emit_body(stmt.then_branch);
    if (!stmt.else_branch.empty()) {
        out_.line("} else {");
        emit_body(stmt.else_branch);
    }
    out_.line("}");
}

void Generator::emit_enquanto(const EnquantoStmt& stmt) {
    const std::string test = condition(stmt.condition);
    if (failed()) return;
    out_.line("while (" + test + ") {");
    emit_body(stmt.body);
    out_.line("}");
}

void Generator::emit_repita(const RepitaStmt& stmt) {
    out_.line("do {");
    emit_body(stmt.body);
    const std::string test = condition(stmt.condition);
    if (failed()) return;
    out_.line("} while (!(" + test + "));");
}

void Generator::emit_para(const ParaStmt& stmt) {
    const std::string counter = c_name(stmt.var_name);
    double step_value = 1.0;
    double ignored = 0.0;
    const bool step_known = !stmt.step || constant_int(stmt.step, step_value);
    const bool end_known = constant_int(stmt.to, ignored);
    const bool needs_block = !step_known || !end_known;
    const std::string id = needs_block ? std::to_string(next_loop_id()) : "";

    std::string end_text = expr_as(stmt.to, TypeKind::Integer);
    std::string step_text = stmt.step ? expr_as(stmt.step, TypeKind::Integer) : "1LL";
    const std::string start_text = expr_as(stmt.from, TypeKind::Integer);
    if (failed()) return;

    if (needs_block) {
        out_.line("{");
        out_.indent();
    }
    if (!end_known) {
        out_.line("spike_int spike_to_" + id + " = " + end_text + ";");
        end_text = "spike_to_" + id;
    }
    if (!step_known) {
        out_.line("spike_int spike_step_" + id + " = " + step_text + ";");
        step_text = "spike_step_" + id;
        out_.line("spike_check_step(" + step_text + ", " + std::to_string(stmt.line) + ");");
    }
    out_.line(counter + " = " + start_text + ";");

    std::string test;
    if (step_known) {
        test = counter + (step_value > 0 ? " <= " : " >= ") + end_text;
    } else {
        test = "(" + step_text + " > 0) ? (" + counter + " <= " + end_text + ") : (" + counter +
               " >= " + end_text + ")";
    }
    out_.line("while (" + test + ") {");
    out_.indent();
    emit_block(stmt.body);
    out_.line("if (!spike_for_advance(&" + counter + ", " + step_text + ")) break;");
    out_.dedent();
    out_.line("}");
    if (needs_block) {
        out_.dedent();
        out_.line("}");
    }
}

} // namespace spike::codegen_detail
