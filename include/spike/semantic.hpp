#ifndef SPIKE_SEMANTIC_HPP
#define SPIKE_SEMANTIC_HPP

#include "spike/ast.hpp"
#include "spike/const_value.hpp"
#include "spike/diagnostics.hpp"
#include "spike/symbol_table.hpp"

#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace spike {

struct NameRef {
    std::string name;
    int line = 0;
    int col = 0;
};

struct LoopVarWrite {
    std::string name;
    int line = 0;
    int col = 0;
};

struct ProcedureSignature {
    std::string name;
    std::vector<Param> params;
    int line = 0;
    int col = 0;
    TypeKind return_type = TypeKind::None;
};

class SemanticAnalyzer {
public:
    SemanticAnalyzer(const Program& program, const std::string& filename,
                     const std::string& source);

    void analyze();

    bool had_error() const;
    const Diagnostics& diagnostics() const;
    const SymbolTable& symbols() const;
    const std::unordered_map<const Expr*, TypeKind>& expr_types() const;
    const std::vector<LoopVarWrite>& loop_var_writes() const;
    const ConstantMap& constants() const;
    const std::vector<NameRef>& functions_without_return() const;

private:
    const Program& program_;
    Diagnostics diags_;
    SymbolTable symbols_;
    std::unordered_map<const Expr*, TypeKind> expr_types_;
    std::set<std::pair<std::string, int>> reported_undeclared_;
    std::vector<std::string> active_loop_vars_;
    int loop_depth_ = 0;
    std::vector<LoopVarWrite> loop_var_writes_;
    ConstantMap constants_;
    std::unordered_map<std::string, ProcedureSignature> procedures_;
    const ProcedureDecl* current_subprogram_ = nullptr;
    std::vector<NameRef> functions_without_return_;

    void declare_consts();
    void declare_const(const ConstDecl& decl);
    std::optional<NameRef> first_non_constant(const ExprPtr& expr) const;
    bool resolve_dims(const VarDecl& decl, std::vector<ArrayBounds>& dims, bool local);
    long long bound_value(const ArrayBound& bound, bool& ok);
    TypeKind check_index(const IndexExpr& node, bool reading = true);
    Symbol* check_target(const ExprPtr& target);
    bool declared_anywhere(const std::string& name) const;
    void declare_globals();
    void declare_variable(const VarDecl& decl, bool local);
    void declare_procedures();
    void check_procedure(const ProcedureDecl& proc);
    void check_call(const CallStmt& call);
    TypeKind check_call_expr(const CallExpr& call);
    void check_arguments(const ProcedureSignature& signature, const std::vector<ExprPtr>& args,
                         int line, int col);
    TypeKind check_argument_value(const ExprPtr& arg);
    void check_ref_argument(const Param& param, const ExprPtr& arg);
    void check_return(const ReturnStmt& stmt);
    Symbol* resolve(const std::string& name, int line, int col);
    Symbol* mark_assigned(const std::string& name, int line, int col);

    void check_block(const StmtList& stmts);
    void check_loop_body(const StmtList& stmts);
    void check_stmt(const Stmt& stmt);
    void check_assign(const AssignStmt& stmt);
    void check_condition(const ExprPtr& condition, const char* command);
    void check_para(const ParaStmt& stmt);
    void check_escolha(const EscolhaStmt& stmt);
    TypeKind check_case_value(const ExprPtr& value, TypeKind selector);
    void check_para_bound(const ExprPtr& bound, const char* part);
    TypeKind check_expr(const ExprPtr& expr);
    TypeKind check_binary(const BinaryExpr& node, TypeKind left, TypeKind right);
    TypeKind check_unary(const UnaryExpr& node, TypeKind operand);
};

} // namespace spike

#endif // SPIKE_SEMANTIC_HPP
