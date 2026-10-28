#ifndef SPIKE_CODEGEN_INTERNAL_HPP
#define SPIKE_CODEGEN_INTERNAL_HPP

#include "spike/ast.hpp"
#include "spike/codegen.hpp"
#include "spike/symbol_table.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace spike::codegen_detail {

using TypeMap = std::unordered_map<const Expr*, TypeKind>;

class CWriter {
public:
    void line(const std::string& text);
    void directive(const std::string& text);
    void blank();
    void raw(const std::string& text);
    void indent();
    void dedent();
    const std::string& str() const;
    int next_line() const;

private:
    std::string out_;
    int depth_ = 0;
};

std::string c_name(const std::string& name);
std::string procedure_name(const std::string& name);
std::string function_name(const std::string& name);
std::string int_literal(double value);
std::string integer_literal(long long value);
std::string constant_literal(const ConstValue& value);
std::string real_literal(double value);
std::string string_literal(const std::string& value);
std::string comment_safe(const std::string& text);
const char* c_type(TypeKind type);
const char* default_value(TypeKind type);

struct Access {
    bool reads = false;
    bool changes = false;
};

Access operator|(Access a, Access b);

class Generator {
public:
    Generator(const Program& program, const TypeMap& types, const ConstantMap& constants,
              std::string source_name, CodegenOptions options);
    CodegenResult run();

private:
    const Program& program_;
    const TypeMap& types_;
    const ConstantMap& constants_;
    std::string source_name_;
    CodegenOptions options_;
    bool in_source_lines_ = false;
    std::unordered_map<std::string, TypeKind> var_types_;
    std::unordered_map<std::string, std::vector<ArrayBounds>> var_dims_;
    std::unordered_map<std::string, const ProcedureDecl*> procedures_;
    const ProcedureDecl* current_ = nullptr;
    std::unordered_set<std::string> ref_params_;
    std::unordered_set<std::string> with_effects_;
    std::unordered_set<std::string> locals_;
    std::unordered_set<std::string> locals_by_ref_;
    std::vector<std::string> pending_;
    Access later_;
    CWriter out_;
    CodegenResult result_;
    int loop_count_ = 0;

    bool failed() const;
    void collect_effects();
    bool has_effects(const ProcedureDecl& proc) const;
    void enter_scope(const ProcedureDecl* proc);
    bool may_change(const std::string& name) const;
    Access access(const ExprPtr& value) const;
    Access ref_access(const ExprPtr& arg) const;
    Access arg_access(const ProcedureDecl& proc, const std::vector<ExprPtr>& args,
                      std::size_t from) const;
    bool must_come_first(const Access& part, const Access& after) const;
    std::string temporary(const std::string& type, const std::string& value);
    std::string logical(const BinaryExpr& node);
    std::vector<std::string> take_pending();
    void emit_lines(const std::vector<std::string>& lines);
    void emit_line(const std::string& text);
    void fail(const char* code, const std::string& message, int line, int col);

    void emit_header();
    void emit_source_lines();
    void emit_source_setup();
    void mark_source_line(int line);
    void mark_generated_code();
    void emit_user_block(const StmtList& stmts);
    void emit_constants();
    void emit_globals();
    void emit_main();
    void emit_prototypes();
    void emit_procedures();
    void emit_procedure(const ProcedureDecl& proc);
    void emit_local(const VarDecl& decl);
    void emit_call(const CallStmt& stmt);
    void emit_return(const ReturnStmt& stmt);
    std::string call(const std::string& name, const std::vector<ExprPtr>& args, int line, int col);
    std::string builtin_call(const std::string& name, const std::vector<ExprPtr>& args, int line,
                             int col);
    std::string signature(const ProcedureDecl& proc) const;
    std::string variable(const std::string& name) const;
    std::string reference(const ExprPtr& arg);
    void emit_block(const StmtList& stmts);
    void emit_stmt(const Stmt& stmt);
    void emit_assign(const AssignStmt& stmt);
    void emit_escreva(const EscrevaStmt& stmt);
    void emit_leia(const LeiaStmt& stmt);
    void emit_se(const SeStmt& stmt);
    void emit_enquanto(const EnquantoStmt& stmt);
    void emit_repita(const RepitaStmt& stmt);
    void emit_para(const ParaStmt& stmt);
    void emit_escolha(const EscolhaStmt& stmt);
    std::string case_condition(const CaseClause& clause, const std::string& selector,
                               TypeKind type);
    void emit_body(const StmtList& stmts);
    std::string condition(const ExprPtr& value);
    int next_loop_id();

    TypeKind type_of(const Expr& expr);
    std::string expr(const ExprPtr& expr);
    long long bound_value(const ArrayBound& bound) const;
    void collect_array_dims();
    std::vector<ArrayBounds> dims_of(const VarDecl& decl) const;
    long long element_count(const std::string& name) const;
    std::string element(const IndexExpr& node);
    std::string lvalue(const ExprPtr& target, std::string& name);
    std::string expr_as(const ExprPtr& expr, TypeKind wanted, const Access& after = Access{});
    std::string ordered_reference(const ExprPtr& arg, const Access& after);
    std::string binary(const BinaryExpr& node, TypeKind result);
    std::string unary(const UnaryExpr& node);
};

} // namespace spike::codegen_detail

#endif // SPIKE_CODEGEN_INTERNAL_HPP
