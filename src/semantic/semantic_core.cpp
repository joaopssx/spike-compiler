#include "spike/semantic.hpp"

#include <string>

namespace spike {

SemanticAnalyzer::SemanticAnalyzer(const Program& program, const std::string& filename,
                                   const std::string& source)
    : program_(program), diags_(filename, source) {}

void SemanticAnalyzer::analyze() {
    declare_consts();
    declare_globals();
    declare_procedures();
    for (const ProcedureDecl& proc : program_.procedures) check_procedure(proc);
    check_block(program_.body);
}

bool SemanticAnalyzer::had_error() const {
    return diags_.has_errors();
}

const Diagnostics& SemanticAnalyzer::diagnostics() const {
    return diags_;
}

const SymbolTable& SemanticAnalyzer::symbols() const {
    return symbols_;
}

const std::unordered_map<const Expr*, TypeKind>& SemanticAnalyzer::expr_types() const {
    return expr_types_;
}

const std::vector<LoopVarWrite>& SemanticAnalyzer::loop_var_writes() const {
    return loop_var_writes_;
}

const ConstantMap& SemanticAnalyzer::constants() const {
    return constants_;
}

void SemanticAnalyzer::declare_globals() {
    for (const VarDecl& decl : program_.globals) declare_variable(decl, false);
}

void SemanticAnalyzer::declare_variable(const VarDecl& decl, bool local) {
    std::vector<ArrayBounds> dims;
    const bool dims_ok = resolve_dims(decl, dims, local);
    const TypeKind type = dims_ok ? decl.type : TypeKind::Invalid;
    if (symbols_.declare(decl.name, type, decl.line, decl.col)) {
        symbols_.lookup(decl.name)->dims = std::move(dims);
        return;
    }
    const Symbol* first = symbols_.lookup(decl.name);
    diags_.error("E701",
                 first->is_const ? "'" + decl.name + "' já foi declarada como constante"
                                 : "variável '" + decl.name + "' já foi declarada",
                 decl.line, decl.col,
                 "a primeira declaração de '" + first->name + "' está na linha " +
                     std::to_string(first->line) + ". Apague uma das duas.");
}

Symbol* SemanticAnalyzer::resolve(const std::string& name, int line, int col) {
    Symbol* symbol = symbols_.lookup(name);
    if (symbol) return symbol;
    if (!reported_undeclared_.insert({symbol_key(name), line}).second) return nullptr;
    if (procedures_.count(symbol_key(name)) != 0) {
        diags_.error("E784", "'" + name + "' é um procedimento, não uma variável", line, col,
                     "para chamar o procedimento, escreva " + name + "(...) sozinho na linha.");
        return nullptr;
    }
    diags_.error("E700", "variável '" + name + "' não foi declarada", line, col,
                 "declare '" + name + "' na seção var.");
    return nullptr;
}

Symbol* SemanticAnalyzer::mark_assigned(const std::string& name, int line, int col) {
    Symbol* symbol = resolve(name, line, col);
    if (!symbol) return nullptr;
    if (symbol->is_const) {
        diags_.error("E740", "não é possível alterar a constante '" + symbol->name + "'", line, col,
                     "se o valor precisa mudar, declare '" + symbol->name + "' na seção var.");
        return nullptr;
    }
    symbol->assigned = true;
    const std::string key = symbol_key(name);
    for (const std::string& loop_var : active_loop_vars_) {
        if (loop_var == key) {
            loop_var_writes_.push_back(LoopVarWrite{name, line, col});
            break;
        }
    }
    return symbol;
}

} // namespace spike
