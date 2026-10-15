#ifndef SPIKE_CODEGEN_INTERNAL_HPP
#define SPIKE_CODEGEN_INTERNAL_HPP

#include "spike/ast.hpp"
#include "spike/codegen.hpp"

#include <string>
#include <unordered_map>

namespace spike::codegen_detail {

using TypeMap = std::unordered_map<const Expr*, TypeKind>;

class CWriter {
public:
    void line(const std::string& text);
    void blank();
    void raw(const std::string& text);
    void indent();
    void dedent();
    const std::string& str() const;

private:
    std::string out_;
    int depth_ = 0;
};

std::string c_name(const std::string& name);
std::string int_literal(double value);
std::string real_literal(double value);
std::string string_literal(const std::string& value);
std::string comment_safe(const std::string& text);
const char* c_type(TypeKind type);
const char* default_value(TypeKind type);

class Generator {
public:
    Generator(const Program& program, const TypeMap& types, std::string source_name);
    CodegenResult run();

private:
    const Program& program_;
    const TypeMap& types_;
    std::string source_name_;
    std::unordered_map<std::string, TypeKind> var_types_;
    CWriter out_;
    CodegenResult result_;

    bool failed() const;
    void fail(const char* code, const std::string& message, int line, int col);

    void emit_header();
    void emit_globals();
    void emit_main();
    void emit_block(const StmtList& stmts);
    void emit_stmt(const Stmt& stmt);
    void emit_assign(const AssignStmt& stmt);
    void emit_escreva(const EscrevaStmt& stmt);

    TypeKind type_of(const Expr& expr);
    std::string expr(const ExprPtr& expr);
    std::string expr_as(const ExprPtr& expr, TypeKind wanted);
    std::string binary(const BinaryExpr& node, TypeKind result);
    std::string unary(const UnaryExpr& node);
};

} // namespace spike::codegen_detail

#endif // SPIKE_CODEGEN_INTERNAL_HPP
