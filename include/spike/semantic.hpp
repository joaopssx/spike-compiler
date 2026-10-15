#ifndef SPIKE_SEMANTIC_HPP
#define SPIKE_SEMANTIC_HPP

#include "spike/ast.hpp"
#include "spike/diagnostics.hpp"
#include "spike/symbol_table.hpp"

#include <set>
#include <string>
#include <unordered_map>
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
    const std::unordered_map<const Expr*, TypeKind>& expr_types() const;

private:
    const Program& program_;
    Diagnostics diags_;
    SymbolTable symbols_;
    std::unordered_map<const Expr*, TypeKind> expr_types_;
    std::set<std::pair<std::string, int>> reported_undeclared_;

    void declare_globals();
    Symbol* resolve(const std::string& name, int line, int col);
    void mark_assigned(const std::string& name, int line, int col);

    void check_block(const StmtList& stmts);
    void check_stmt(const Stmt& stmt);
    void check_assign(const AssignStmt& stmt);
    TypeKind check_expr(const ExprPtr& expr);
    TypeKind check_binary(const BinaryExpr& node, TypeKind left, TypeKind right);
    TypeKind check_unary(const UnaryExpr& node, TypeKind operand);
};

} // namespace spike

#endif // SPIKE_SEMANTIC_HPP
