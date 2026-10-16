#include "spike/semantic.hpp"
#include "semantic/semantic_internal.hpp"

#include <string>
#include <utility>
#include <vector>

namespace spike {

using semantic_detail::expr_start;
using semantic_detail::quoted;

namespace {

std::string argument_count(std::size_t count) {
    return std::to_string(count) + (count == 1 ? " argumento" : " argumentos");
}

bool accepts(TypeKind param, TypeKind value) {
    return param == value || (param == TypeKind::Real && value == TypeKind::Integer);
}

std::string line_text(int line) {
    return std::to_string(line);
}

} // namespace

void SemanticAnalyzer::declare_procedures() {
    for (const ProcedureDecl& proc : program_.procedures) {
        if (proc.name.empty()) continue;
        const std::string key = symbol_key(proc.name);
        const auto it = procedures_.find(key);
        if (it != procedures_.end()) {
            diags_.error("E783", "procedimento '" + proc.name + "' já foi declarado", proc.line,
                         proc.col,
                         "a primeira declaração de '" + it->second.name + "' está na linha " +
                             line_text(it->second.line) + ". Apague ou renomeie uma delas.");
            continue;
        }
        if (const Symbol* other = symbols_.lookup_global(proc.name)) {
            diags_.error("E783",
                         "'" + proc.name + "' já foi declarada como " +
                             (other->is_const ? "constante" : "variável"),
                         proc.line, proc.col,
                         "a declaração de '" + other->name + "' está na linha " +
                             line_text(other->line) + ". Use outro nome para o procedimento.");
            continue;
        }
        procedures_.emplace(key, ProcedureSignature{proc.name, proc.params, proc.line, proc.col});
    }
}

void SemanticAnalyzer::check_procedure(const ProcedureDecl& proc) {
    symbols_.push_scope();
    const int saved_depth = loop_depth_;
    std::vector<std::string> saved_loop_vars = std::move(active_loop_vars_);
    loop_depth_ = 0;
    active_loop_vars_.clear();
    for (const Param& param : proc.params) {
        if (symbols_.declare(param.name, param.type, param.line, param.col)) {
            symbols_.lookup(param.name)->assigned = true;
            continue;
        }
        const Symbol* first = symbols_.lookup(param.name);
        diags_.error("E701", "parâmetro '" + param.name + "' repetido", param.line, param.col,
                     "o primeiro '" + first->name + "' está na linha " + line_text(first->line) +
                         ". Use um nome diferente para cada parâmetro.");
    }
    for (const VarDecl& decl : proc.locals) declare_variable(decl, true);
    check_block(proc.body);
    loop_depth_ = saved_depth;
    active_loop_vars_ = std::move(saved_loop_vars);
    symbols_.pop_scope();
}

void SemanticAnalyzer::check_call(const CallStmt& call) {
    std::vector<TypeKind> types;
    for (const ExprPtr& arg : call.args) types.push_back(check_expr(arg));
    const auto it = procedures_.find(symbol_key(call.name));
    if (it == procedures_.end()) {
        const bool is_variable = symbols_.lookup(call.name) != nullptr;
        diags_.error("E780", "procedimento '" + call.name + "' não foi definido", call.line,
                     call.col,
                     is_variable ? "'" + call.name +
                                       "' é uma variável; só procedimentos são chamados com ( )."
                                 : "declare 'procedimento " + call.name +
                                       "(...)' antes do 'inicio' do programa.");
        return;
    }
    const ProcedureSignature& signature = it->second;
    if (types.size() != signature.params.size()) {
        diags_.error("E781",
                     "'" + signature.name + "' espera " +
                         argument_count(signature.params.size()) + ", recebeu " +
                         std::to_string(types.size()),
                     call.line, call.col,
                     "a declaração de '" + signature.name + "' está na linha " +
                         line_text(signature.line) + ".");
        return;
    }
    for (std::size_t i = 0; i < types.size(); ++i) {
        const Param& param = signature.params[i];
        if (!call.args[i] || types[i] == TypeKind::Invalid || accepts(param.type, types[i])) {
            continue;
        }
        const SourcePos pos = expr_start(*call.args[i]);
        diags_.error("E782",
                     "o argumento " + std::to_string(i + 1) + " de '" + signature.name +
                         "' precisa ser " + quoted(param.type) + ", mas é " + quoted(types[i]),
                     pos.line, pos.col,
                     param.type == TypeKind::Integer && types[i] == TypeKind::Real
                         ? "o parâmetro '" + param.name +
                               "' é inteiro; use trunca() ou arredonda() no valor."
                         : "o parâmetro '" + param.name + "' é " + quoted(param.type) +
                               "; passe um valor desse tipo.");
    }
}

} // namespace spike
