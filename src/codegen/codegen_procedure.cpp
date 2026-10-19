#include "codegen/codegen_internal.hpp"
#include "spike/symbol_table.hpp"

#include <string>
#include <utility>

namespace spike::codegen_detail {

namespace {

std::string subprogram_name(const ProcedureDecl& proc) {
    return proc.is_function() ? function_name(proc.name) : procedure_name(proc.name);
}

} // namespace

std::string Generator::signature(const ProcedureDecl& proc) const {
    std::string params;
    for (const Param& param : proc.params) {
        const char* type = c_type(param.type);
        if (!params.empty()) params += ", ";
        params += std::string(type ? type : "int") + " " + c_name(param.name);
    }
    const char* result = proc.is_function() ? c_type(proc.return_type) : "void";
    return std::string("static ") + (result ? result : "int") + " " + subprogram_name(proc) +
           "(" + (params.empty() ? "void" : params) + ")";
}

void Generator::emit_prototypes() {
    if (program_.procedures.empty()) return;
    for (const ProcedureDecl& proc : program_.procedures) out_.line(signature(proc) + ";");
    out_.blank();
}

void Generator::emit_procedures() {
    for (const ProcedureDecl& proc : program_.procedures) {
        if (failed()) return;
        emit_procedure(proc);
    }
}

void Generator::emit_procedure(const ProcedureDecl& proc) {
    const auto saved_types = var_types_;
    const auto saved_dims = var_dims_;
    for (const Param& param : proc.params) {
        const std::string key = symbol_key(param.name);
        var_types_[key] = param.type;
        var_dims_.erase(key);
    }
    for (const VarDecl& decl : proc.locals) {
        const std::string key = symbol_key(decl.name);
        var_types_[key] = decl.type;
        if (decl.dims.empty()) {
            var_dims_.erase(key);
        } else {
            var_dims_[key] = dims_of(decl);
        }
    }
    current_ = &proc;
    out_.line(signature(proc) + " {");
    out_.indent();
    out_.line("spike_enter(" + std::to_string(proc.line) + ");");
    for (const VarDecl& decl : proc.locals) emit_local(decl);
    emit_block(proc.body);
    if (proc.is_function()) {
        const int end_line = proc.end_line != 0 ? proc.end_line : proc.line;
        out_.line("spike_missing_return(" + std::to_string(end_line) + ", " +
                  string_literal(proc.name) + ");");
        const char* fallback = default_value(proc.return_type);
        out_.line(std::string("return ") + (fallback ? fallback : "0") + ";");
    } else {
        out_.line("spike_leave();");
    }
    out_.dedent();
    out_.line("}");
    out_.blank();
    current_ = nullptr;
    var_types_ = saved_types;
    var_dims_ = saved_dims;
}

void Generator::emit_local(const VarDecl& decl) {
    const char* type = c_type(decl.type);
    if (!type) {
        fail("E900", "erro interno do gerador: variável '" + decl.name + "' sem tipo", decl.line,
             decl.col);
        return;
    }
    const std::string name = c_name(decl.name);
    if (decl.dims.empty()) {
        out_.line(std::string(type) + " " + name + " = " + default_value(decl.type) + ";");
        return;
    }
    const std::string count = std::to_string(element_count(decl.name));
    if (decl.type == TypeKind::String) {
        out_.line(std::string(type) + " " + name + "[" + count + "];");
        out_.line("spike_fill_str(" + name + ", " + count + ");");
        return;
    }
    out_.line(std::string(type) + " " + name + "[" + count + "] = {0};");
}

std::string Generator::call(const std::string& name, const std::vector<ExprPtr>& args, int line,
                            int col) {
    const auto it = procedures_.find(symbol_key(name));
    if (it == procedures_.end() || it->second->params.size() != args.size()) {
        fail("E900", "erro interno do gerador: subprograma '" + name + "' desconhecido", line,
             col);
        return "";
    }
    const ProcedureDecl& proc = *it->second;
    std::string text;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (i > 0) text += ", ";
        text += expr_as(args[i], proc.params[i].type);
    }
    return subprogram_name(proc) + "(" + text + ")";
}

void Generator::emit_call(const CallStmt& stmt) {
    const std::string text = call(stmt.name, stmt.args, stmt.line, stmt.col);
    if (failed()) return;
    out_.line(text + ";");
}

void Generator::emit_return(const ReturnStmt& stmt) {
    if (!current_) {
        fail("E900", "erro interno do gerador: 'retorne' fora de subprograma", stmt.line,
             stmt.col);
        return;
    }
    if (!current_->is_function()) {
        out_.line("spike_leave();");
        out_.line("return;");
        return;
    }
    const char* type = c_type(current_->return_type);
    const std::string value = expr_as(stmt.value, current_->return_type);
    if (failed() || !type) return;
    out_.line("{");
    out_.indent();
    out_.line(std::string(type) + " spike_result = " + value + ";");
    out_.line("spike_leave();");
    out_.line("return spike_result;");
    out_.dedent();
    out_.line("}");
}

} // namespace spike::codegen_detail
