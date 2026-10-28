#include "codegen/codegen_internal.hpp"

#include <string>
#include <variant>

namespace spike::codegen_detail {

std::string Generator::case_condition(const CaseClause& clause, const std::string& selector,
                                      TypeKind type) {
    std::string condition;
    for (const CaseLabel& label : clause.labels) {
        const std::string value = expr(label.value);
        std::string test;
        if (type == TypeKind::String) {
            test = "spike_str_eq(" + selector + ", " + value + ")";
        } else if (label.range_end) {
            test = selector + " >= " + value + " && " + selector + " <= " + expr(label.range_end);
            if (clause.labels.size() > 1) test = "(" + test + ")";
        } else {
            test = selector + " == " + value;
        }
        condition += (condition.empty() ? "" : " || ") + test;
    }
    return condition;
}

void Generator::emit_escolha(const EscolhaStmt& stmt) {
    if (!stmt.selector) {
        fail("E900", "erro interno do gerador: 'escolha' sem valor", stmt.line, stmt.col);
        return;
    }
    const TypeKind type = type_of(*stmt.selector);
    const std::string selector_text = expr(stmt.selector);
    if (failed()) return;

    const bool needs_temp = !std::holds_alternative<VarExpr>(stmt.selector->node);
    std::string selector = selector_text;
    if (needs_temp) {
        selector = "spike_sel_" + std::to_string(next_loop_id());
        out_.line("{");
        out_.indent();
        emit_lines(take_pending());
        out_.line(std::string(c_type(type)) + " " + selector + " = " + selector_text + ";");
    }

    bool first = true;
    for (const CaseClause& clause : stmt.cases) {
        const std::string condition = case_condition(clause, selector, type);
        if (failed()) return;
        out_.line(std::string(first ? "if (" : "} else if (") + condition + ") {");
        emit_body(clause.body);
        first = false;
    }
    if (stmt.has_default) {
        if (first) {
            emit_block(stmt.default_body);
        } else {
            out_.line("} else {");
            emit_body(stmt.default_body);
        }
    }
    if (!first) out_.line("}");

    if (needs_temp) {
        out_.dedent();
        out_.line("}");
    }
}

} // namespace spike::codegen_detail
