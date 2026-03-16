#ifndef SPIKE_AST_HPP
#define SPIKE_AST_HPP

#include <optional>
#include <string>
#include <variant>
#include <vector>

struct IdentifierExpr {
    std::string name;
};

struct IntegerLiteralExpr {
    std::string value;
};

struct RealLiteralExpr {
    std::string value;
};

struct StringLiteralExpr {
    std::string value;
};

using Expression =
    std::variant<IdentifierExpr, IntegerLiteralExpr, RealLiteralExpr, StringLiteralExpr>;

struct VarDecl {
    std::string name;
    std::string type_name;
};

struct AssignStmt {
    std::string target;
    Expression value;
};

struct WriteStmt {
    Expression value;
};

using Statement = std::variant<AssignStmt, WriteStmt>;

struct Program {
    std::optional<std::string> name;
    std::vector<VarDecl> variables;
    std::vector<Statement> statements;
};

std::string expression_to_string(const Expression& expression);
std::string ast_to_string(const Program& program);

#endif
