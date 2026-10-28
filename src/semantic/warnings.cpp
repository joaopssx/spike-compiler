#include "spike/warnings.hpp"
#include "semantic/warnings_internal.hpp"

#include <map>
#include <string>
#include <utility>

namespace spike {

namespace {

using SymbolsByPosition = std::map<std::pair<int, int>, const Symbol*>;

SymbolsByPosition index_symbols(const SymbolTable& symbols) {
    SymbolsByPosition index;
    for (const Symbol& symbol : symbols) index[{symbol.line, symbol.col}] = &symbol;
    return index;
}

void check_variable(const VarDecl& decl, const SymbolsByPosition& symbols, Diagnostics& out) {
    const auto it = symbols.find({decl.line, decl.col});
    if (it == symbols.end()) return;
    const Symbol& symbol = *it->second;
    const std::string name = "'" + decl.name + "'";
    if (!symbol.used && !symbol.assigned) {
        out.warning("W001", "a variável " + name + " foi declarada, mas nunca é usada", decl.line,
                    decl.col, "apague a declaração ou use a variável no programa.");
    } else if (symbol.used && !symbol.assigned) {
        out.warning("W002", "a variável " + name + " é lida, mas nunca recebe valor", decl.line,
                    decl.col,
                    "ela vai valer sempre o valor padrão (0, \"\" ou falso). Guarde um valor com "
                    "'<-' ou 'leia' antes de usar.");
    } else if (!symbol.used && symbol.assigned) {
        out.warning("W003", "a variável " + name + " recebe valor, mas nunca é lida", decl.line,
                    decl.col,
                    "o valor guardado não serve para nada; mostre-o com 'escreva', use-o numa "
                    "conta ou apague a variável.");
    }
}

void check_shadowing(const ProcedureDecl& proc, const SymbolTable& symbols, Diagnostics& out) {
    for (const VarDecl& decl : proc.locals) {
        const Symbol* global = symbols.lookup_global(decl.name);
        if (!global) continue;
        out.warning("W005",
                    "a variável local '" + decl.name + "' tem o mesmo nome de uma " +
                        (global->is_const ? "constante global" : "variável global"),
                    decl.line, decl.col,
                    "dentro de '" + proc.name + "', '" + decl.name +
                        "' é a local; a global da linha " + std::to_string(global->line) +
                        " fica escondida. Se quis usar a global, troque o nome da local.");
    }
}

void check_loop_var_writes(const SemanticAnalyzer& semantic, Diagnostics& out) {
    for (const LoopVarWrite& write : semantic.loop_var_writes()) {
        out.warning("W004",
                    "a variável '" + write.name + "' do 'para' é alterada dentro do laço",
                    write.line, write.col,
                    "o 'para' já muda '" + write.name +
                        "' a cada volta; alterar o valor aqui muda quantas voltas ele dá. Use "
                        "outra variável ou troque o 'para' por um 'enquanto'.");
    }
}

void check_missing_returns(const SemanticAnalyzer& semantic, Diagnostics& out) {
    for (const NameRef& function : semantic.functions_without_return()) {
        out.warning("W006", "a função '" + function.name + "' pode terminar sem 'retorne'",
                    function.line, function.col,
                    "se a execução chegar ao 'fimfuncao' sem passar por um 'retorne', o programa "
                    "para com o erro R009. Coloque um 'retorne' no fim da função.");
    }
}

} // namespace

void check_warnings(const Program& program, const SemanticAnalyzer& semantic, Diagnostics& out) {
    if (semantic.had_error()) return;
    const SymbolsByPosition symbols = index_symbols(semantic.symbols());
    for (const VarDecl& decl : program.globals) check_variable(decl, symbols, out);
    for (const ProcedureDecl& proc : program.procedures) {
        for (const VarDecl& decl : proc.locals) check_variable(decl, symbols, out);
        check_shadowing(proc, semantic.symbols(), out);
    }
    check_loop_var_writes(semantic, out);
    check_missing_returns(semantic, out);
    warnings_detail::check_flow_warnings(program, out);
}

} // namespace spike
