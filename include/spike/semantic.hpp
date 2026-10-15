#ifndef SPIKE_SEMANTIC_HPP
#define SPIKE_SEMANTIC_HPP

#include "spike/ast.hpp"
#include "spike/diagnostics.hpp"
#include "spike/symbol_table.hpp"

#include <set>
#include <string>
#include <utility>

namespace spike {

class SemanticAnalyzer {
public:
    SemanticAnalyzer(const Program& program, const std::string& filename,
                     const std::string& source);

    void analyze();

    bool had_error() const;
    const Diagnostics& diagnostics() const;
    const SymbolTable& symbols() const;

private:
    const Program& program_;
    Diagnostics diags_;
    SymbolTable symbols_;
    std::set<std::pair<std::string, int>> reported_undeclared_;

    void declare_globals();
    Symbol* resolve(const std::string& name, int line, int col);
    void mark_read(const std::string& name, int line, int col);
    void mark_assigned(const std::string& name, int line, int col);

    void check_block(const StmtList& stmts);
    void check_stmt(const Stmt& stmt);
    void check_expr(const ExprPtr& expr);
};

} // namespace spike

#endif // SPIKE_SEMANTIC_HPP
