#include "spike/builtins.hpp"
#include "spike/semantic.hpp"
#include "semantic/semantic_internal.hpp"

#include <string>
#include <variant>
#include <vector>

namespace spike {

using semantic_detail::expr_start;
using semantic_detail::quoted;

namespace {

constexpr const char* kHintVectorArgument =
    "declare o vetor no var do programa; procedimentos e funções enxergam as variáveis globais.";

std::string argument_count(std::size_t count) {
    return std::to_string(count) + (count == 1 ? " argumento" : " argumentos");
}

bool accepts(TypeKind wanted, TypeKind value) {
    return wanted == value || (wanted == TypeKind::Real && value == TypeKind::Integer);
}

} // namespace

TypeKind SemanticAnalyzer::check_argument_value(const ExprPtr& arg) {
    const auto* var = arg ? std::get_if<VarExpr>(&arg->node) : nullptr;
    Symbol* symbol = var ? symbols_.lookup(var->name) : nullptr;
    if (!symbol || symbol->dims.empty()) return check_expr(arg);
    symbol->used = true;
    diags_.error("E788", "vetores ainda não podem ser passados a procedimentos ou funções",
                 var->line, var->col, kHintVectorArgument);
    expr_types_[arg.get()] = TypeKind::Invalid;
    return TypeKind::Invalid;
}

void SemanticAnalyzer::check_ref_argument(const Param& param, const ExprPtr& arg) {
    if (!arg) return;
    const auto* var = std::get_if<VarExpr>(&arg->node);
    const bool is_element = std::holds_alternative<IndexExpr>(arg->node);
    if (!var && !is_element) {
        check_expr(arg);
        const SourcePos pos = expr_start(*arg);
        diags_.error("E785",
                     "o argumento de '" + param.name +
                         "' precisa ser uma variável, porque o parâmetro é por referência",
                     pos.line, pos.col,
                     "guarde o valor numa variável e passe a variável, ou tire o 'var' do "
                     "parâmetro.");
        return;
    }
    const Symbol* named = var ? symbols_.lookup(var->name) : nullptr;
    if (named && named->is_const) {
        expr_types_[arg.get()] = named->type;
        diags_.error("E786",
                     "'" + named->name + "' é uma constante e não pode ir para o parâmetro '" +
                         param.name + "', que é por referência",
                     var->line, var->col,
                     "um parâmetro com 'var' pode mudar o valor, e constantes nunca mudam; copie "
                     "o valor para uma variável.");
        return;
    }
    if (named && !named->dims.empty()) {
        check_argument_value(arg);
        return;
    }
    Symbol* target = check_target(arg);
    if (!target) {
        expr_types_[arg.get()] = TypeKind::Invalid;
        return;
    }
    target->used = true;
    expr_types_[arg.get()] = target->type;
    if (target->type == TypeKind::Invalid || param.type == TypeKind::Invalid ||
        target->type == param.type) {
        return;
    }
    const SourcePos pos = expr_start(*arg);
    diags_.error("E787",
                 "o parâmetro '" + param.name + "' é por referência e exige " +
                     quoted(param.type) + ", mas recebeu " + quoted(target->type),
                 pos.line, pos.col,
                 "com 'var', o tipo precisa ser exatamente o mesmo; passe uma variável do tipo " +
                     quoted(param.type) + ".");
}

void SemanticAnalyzer::check_arguments(const ProcedureSignature& signature,
                                       const std::vector<ExprPtr>& args, int line, int col) {
    std::vector<TypeKind> types;
    for (std::size_t i = 0; i < args.size(); ++i) {
        const Param* param = i < signature.params.size() ? &signature.params[i] : nullptr;
        if (param && param->by_ref && param->dims.empty()) {
            check_ref_argument(*param, args[i]);
            types.push_back(TypeKind::Invalid);
        } else {
            types.push_back(check_argument_value(args[i]));
        }
    }
    if (args.size() != signature.params.size()) {
        diags_.error("E781",
                     "'" + signature.name + "' espera " +
                         argument_count(signature.params.size()) + ", recebeu " +
                         std::to_string(args.size()),
                     line, col,
                     "a declaração de '" + signature.name + "' está na linha " +
                         std::to_string(signature.line) + ".");
        return;
    }
    for (std::size_t i = 0; i < types.size(); ++i) {
        const Param& param = signature.params[i];
        if (!args[i] || types[i] == TypeKind::Invalid || !param.dims.empty() ||
            accepts(param.type, types[i])) {
            continue;
        }
        const SourcePos pos = expr_start(*args[i]);
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

void SemanticAnalyzer::check_call(const CallStmt& call) {
    const auto it = procedures_.find(symbol_key(call.name));
    if (it == procedures_.end() && is_builtin_name(call.name)) {
        check_builtin_call(call.name, call.args, call.line, call.col);
        return;
    }
    if (it == procedures_.end()) {
        for (const ExprPtr& arg : call.args) check_expr(arg);
        const bool is_variable = symbols_.lookup(call.name) != nullptr;
        diags_.error("E780", "procedimento '" + call.name + "' não foi definido", call.line,
                     call.col,
                     is_variable ? "'" + call.name +
                                       "' é uma variável; só procedimentos e funções são "
                                       "chamados com ( )."
                                 : "declare 'procedimento " + call.name +
                                       "(...)' antes do 'inicio' do programa.");
        return;
    }
    check_arguments(it->second, call.args, call.line, call.col);
}

TypeKind SemanticAnalyzer::check_call_expr(const CallExpr& call) {
    const auto it = procedures_.find(symbol_key(call.name));
    if (it == procedures_.end() && is_builtin_name(call.name)) {
        return check_builtin_call(call.name, call.args, call.line, call.col);
    }
    if (it == procedures_.end()) {
        for (const ExprPtr& arg : call.args) check_expr(arg);
        const bool is_variable = symbols_.lookup(call.name) != nullptr;
        diags_.error("E780", "função '" + call.name + "' não foi definida", call.line, call.col,
                     is_variable ? "'" + call.name +
                                       "' é uma variável; para um elemento de vetor, use [ ]."
                                 : "declare 'funcao " + call.name +
                                       "(...): tipo' antes do 'inicio' do programa.");
        return TypeKind::Invalid;
    }
    const ProcedureSignature& signature = it->second;
    if (signature.return_type == TypeKind::None) {
        for (const ExprPtr& arg : call.args) check_expr(arg);
        diags_.error("E784", "'" + signature.name + "' é um procedimento e não devolve valor",
                     call.line, call.col,
                     "chame " + signature.name +
                         "(...) sozinho na linha, ou transforme o procedimento em uma função.");
        return TypeKind::Invalid;
    }
    check_arguments(signature, call.args, call.line, call.col);
    return signature.return_type;
}

} // namespace spike
