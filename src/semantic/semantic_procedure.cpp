#include "spike/semantic.hpp"
#include "semantic/semantic_internal.hpp"

#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace spike {

using semantic_detail::expr_start;
using semantic_detail::quoted;

namespace {

std::string argument_count(std::size_t count) {
    return std::to_string(count) + (count == 1 ? " argumento" : " argumentos");
}

bool accepts(TypeKind wanted, TypeKind value) {
    return wanted == value || (wanted == TypeKind::Real && value == TypeKind::Integer);
}

std::string line_text(int line) {
    return std::to_string(line);
}

std::string kind_text(const ProcedureDecl& proc) {
    return proc.is_function() ? "função" : "procedimento";
}

bool always_returns(const StmtList& stmts);

bool stmt_returns(const Stmt& stmt) {
    return std::visit(
        [](const auto& node) -> bool {
            using Node = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<Node, ReturnStmt>) {
                return true;
            } else if constexpr (std::is_same_v<Node, SeStmt>) {
                return always_returns(node.then_branch) && always_returns(node.else_branch);
            } else if constexpr (std::is_same_v<Node, EscolhaStmt>) {
                if (!node.has_default || !always_returns(node.default_body)) return false;
                for (const CaseClause& clause : node.cases) {
                    if (!always_returns(clause.body)) return false;
                }
                return true;
            } else {
                return false;
            }
        },
        stmt.node);
}

bool always_returns(const StmtList& stmts) {
    for (const StmtPtr& stmt : stmts) {
        if (!stmt || std::holds_alternative<BreakStmt>(stmt->node)) return false;
        if (stmt_returns(*stmt)) return true;
    }
    return false;
}

} // namespace

void SemanticAnalyzer::declare_procedures() {
    for (const ProcedureDecl& proc : program_.procedures) {
        if (proc.name.empty()) continue;
        const std::string key = symbol_key(proc.name);
        const auto it = procedures_.find(key);
        if (it != procedures_.end()) {
            const bool first_is_function = it->second.return_type != TypeKind::None;
            diags_.error("E783",
                         std::string(first_is_function ? "função '" : "procedimento '") +
                             proc.name + (first_is_function ? "' já foi declarada"
                                                            : "' já foi declarado"),
                         proc.line, proc.col,
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
                             line_text(other->line) + ". Use outro nome para " +
                             (proc.is_function() ? "a função." : "o procedimento."));
            continue;
        }
        procedures_.emplace(key, ProcedureSignature{proc.name, proc.params, proc.line, proc.col,
                                                    proc.return_type});
    }
}

void SemanticAnalyzer::check_procedure(const ProcedureDecl& proc) {
    symbols_.push_scope();
    const int saved_depth = loop_depth_;
    std::vector<std::string> saved_loop_vars = std::move(active_loop_vars_);
    loop_depth_ = 0;
    active_loop_vars_.clear();
    current_subprogram_ = &proc;
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
    if (proc.is_function() && !proc.name.empty() && !always_returns(proc.body)) {
        functions_without_return_.push_back(NameRef{proc.name, proc.line, proc.col});
    }
    current_subprogram_ = nullptr;
    loop_depth_ = saved_depth;
    active_loop_vars_ = std::move(saved_loop_vars);
    symbols_.pop_scope();
}

void SemanticAnalyzer::check_arguments(const ProcedureSignature& signature,
                                       const std::vector<ExprPtr>& args,
                                       const std::vector<TypeKind>& types, int line, int col) {
    if (types.size() != signature.params.size()) {
        diags_.error("E781",
                     "'" + signature.name + "' espera " +
                         argument_count(signature.params.size()) + ", recebeu " +
                         std::to_string(types.size()),
                     line, col,
                     "a declaração de '" + signature.name + "' está na linha " +
                         line_text(signature.line) + ".");
        return;
    }
    for (std::size_t i = 0; i < types.size(); ++i) {
        const Param& param = signature.params[i];
        if (!args[i] || types[i] == TypeKind::Invalid || accepts(param.type, types[i])) continue;
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
    std::vector<TypeKind> types;
    for (const ExprPtr& arg : call.args) types.push_back(check_expr(arg));
    const auto it = procedures_.find(symbol_key(call.name));
    if (it == procedures_.end()) {
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
    check_arguments(it->second, call.args, types, call.line, call.col);
}

TypeKind SemanticAnalyzer::check_call_expr(const CallExpr& call) {
    std::vector<TypeKind> types;
    for (const ExprPtr& arg : call.args) types.push_back(check_expr(arg));
    const auto it = procedures_.find(symbol_key(call.name));
    if (it == procedures_.end()) {
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
        diags_.error("E784", "'" + signature.name + "' é um procedimento e não devolve valor",
                     call.line, call.col,
                     "chame " + signature.name +
                         "(...) sozinho na linha, ou transforme o procedimento em uma função.");
        return TypeKind::Invalid;
    }
    check_arguments(signature, call.args, types, call.line, call.col);
    return signature.return_type;
}

void SemanticAnalyzer::check_return(const ReturnStmt& stmt) {
    const TypeKind value = stmt.value ? check_expr(stmt.value) : TypeKind::None;
    if (!current_subprogram_) {
        diags_.error("E791", "'retorne' só pode ser usado dentro de uma função ou procedimento",
                     stmt.line, stmt.col,
                     "o programa principal termina sozinho ao chegar em 'fimalgoritmo'.");
        return;
    }
    const ProcedureDecl& sub = *current_subprogram_;
    const std::string name = "'" + sub.name + "'";
    if (!sub.is_function()) {
        if (!stmt.value) return;
        const SourcePos pos = expr_start(*stmt.value);
        diags_.error("E793", "o procedimento " + name + " não devolve valor", pos.line, pos.col,
                     "use 'retorne' sozinho para sair do procedimento, ou transforme " + name +
                         " em uma função.");
        return;
    }
    if (!stmt.value) {
        diags_.error("E793", "a função " + name + " precisa devolver um valor", stmt.line,
                     stmt.col, "escreva o valor depois de 'retorne': retorne resultado.");
        return;
    }
    const TypeKind declared = sub.return_type;
    if (value == TypeKind::Invalid || declared == TypeKind::Invalid || accepts(declared, value)) {
        return;
    }
    const SourcePos pos = expr_start(*stmt.value);
    diags_.error("E790",
                 "a " + kind_text(sub) + " " + name + " devolve " + quoted(declared) +
                     ", mas o valor é " + quoted(value),
                 pos.line, pos.col,
                 declared == TypeKind::Integer && value == TypeKind::Real
                     ? "use trunca() ou arredonda() para transformar o valor em inteiro."
                     : "o valor do 'retorne' precisa ser do tipo da função.");
}

} // namespace spike
