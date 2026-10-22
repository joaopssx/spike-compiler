#include "codegen/codegen_internal.hpp"
#include "spike/builtins.hpp"
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
        params += std::string(type ? type : "int") + (param.by_ref ? "* " : " ") +
                  c_name(param.name);
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
    const auto saved_refs = ref_params_;
    for (const Param& param : proc.params) {
        const std::string key = symbol_key(param.name);
        var_types_[key] = param.type;
        var_dims_.erase(key);
        if (param.by_ref) {
            ref_params_.insert(key);
        } else {
            ref_params_.erase(key);
        }
    }
    for (const VarDecl& decl : proc.locals) {
        const std::string key = symbol_key(decl.name);
        var_types_[key] = decl.type;
        ref_params_.erase(key);
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
    ref_params_ = saved_refs;
}

std::string Generator::variable(const std::string& name) const {
    if (ref_params_.count(symbol_key(name)) != 0) return "(*" + c_name(name) + ")";
    return c_name(name);
}

std::string Generator::reference(const ExprPtr& arg) {
    if (const VarExpr* var = target_variable(arg)) {
        if (ref_params_.count(symbol_key(var->name)) != 0) return c_name(var->name);
        return "&" + c_name(var->name);
    }
    if (const auto* index = arg ? std::get_if<IndexExpr>(&arg->node) : nullptr) {
        return "&" + element(*index);
    }
    fail("E900", "erro interno do gerador: argumento por referência sem variável", 0, 0);
    return "";
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
    if (it == procedures_.end()) return builtin_call(name, args, line, col);
    if (it->second->params.size() != args.size()) {
        fail("E900", "erro interno do gerador: subprograma '" + name + "' desconhecido", line,
             col);
        return "";
    }
    const ProcedureDecl& proc = *it->second;
    std::string text;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (i > 0) text += ", ";
        const Param& param = proc.params[i];
        text += param.by_ref ? reference(args[i]) : expr_as(args[i], param.type);
    }
    return subprogram_name(proc) + "(" + text + ")";
}

std::string Generator::builtin_call(const std::string& name, const std::vector<ExprPtr>& args,
                                    int line, int col) {
    std::vector<TypeKind> types;
    for (const ExprPtr& arg : args) types.push_back(arg ? type_of(*arg) : TypeKind::Invalid);
    if (failed()) return "";
    const BuiltinSig* sig = find_builtin(symbol_key(name), types);
    if (!sig) {
        fail("E900", "erro interno do gerador: função pronta '" + name + "' sem assinatura", line,
             col);
        return "";
    }
    std::string text;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (i > 0) text += ", ";
        text += expr_as(args[i], sig->params[i]);
    }
    if (sig->takes_line) text += (args.empty() ? "" : ", ") + std::to_string(line);
    return sig->c_name + "(" + text + ")";
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
