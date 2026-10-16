#include "codegen/codegen_internal.hpp"
#include "spike/symbol_table.hpp"

#include <string>
#include <utility>

namespace spike::codegen_detail {

std::string Generator::signature(const ProcedureDecl& proc) const {
    std::string params;
    for (const Param& param : proc.params) {
        const char* type = c_type(param.type);
        if (!params.empty()) params += ", ";
        params += std::string(type ? type : "int") + " " + c_name(param.name);
    }
    return "static void " + procedure_name(proc.name) + "(" + (params.empty() ? "void" : params) +
           ")";
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
    out_.line(signature(proc) + " {");
    out_.indent();
    for (const VarDecl& decl : proc.locals) emit_local(decl);
    emit_block(proc.body);
    out_.dedent();
    out_.line("}");
    out_.blank();
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

void Generator::emit_call(const CallStmt& call) {
    const auto it = procedures_.find(symbol_key(call.name));
    if (it == procedures_.end() || it->second->params.size() != call.args.size()) {
        fail("E900", "erro interno do gerador: procedimento '" + call.name + "' desconhecido",
             call.line, call.col);
        return;
    }
    const ProcedureDecl& proc = *it->second;
    std::string args;
    for (std::size_t i = 0; i < call.args.size(); ++i) {
        if (i > 0) args += ", ";
        args += expr_as(call.args[i], proc.params[i].type);
    }
    if (failed()) return;
    out_.line(procedure_name(proc.name) + "(" + args + ");");
}

} // namespace spike::codegen_detail
