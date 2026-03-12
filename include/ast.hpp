#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "token.hpp"

namespace portugol {

enum class ValueType {
    Inteiro,
    Real,
    Logico,
    Caractere,
    Unknown,
};

inline std::string_view valueTypeName(ValueType type) {
    switch (type) {
    case ValueType::Inteiro:
        return "inteiro";
    case ValueType::Real:
        return "real";
    case ValueType::Logico:
        return "logico";
    case ValueType::Caractere:
        return "caractere";
    case ValueType::Unknown:
        return "desconhecido";
    }

    return "desconhecido";
}

struct Expr {
    explicit Expr(SourceLocation loc)
        : location(loc) {
    }

    virtual ~Expr() = default;

    SourceLocation location;
    ValueType inferredType {ValueType::Unknown};
};

struct IntExpr : Expr {
    IntExpr(SourceLocation loc, int v)
        : Expr(loc)
        , value(v) {
    }

    int value;
};

struct RealExpr : Expr {
    RealExpr(SourceLocation loc, double v)
        : Expr(loc)
        , value(v) {
    }

    double value;
};

struct VariableExpr : Expr {
    VariableExpr(SourceLocation loc, std::string n)
        : Expr(loc)
        , name(std::move(n)) {
    }

    std::string name;
};

struct UnaryExpr : Expr {
    UnaryExpr(SourceLocation loc, std::string expressionOp, std::unique_ptr<Expr> expressionOperand)
        : Expr(loc)
        , op(std::move(expressionOp))
        , operand(std::move(expressionOperand)) {
    }

    std::string op;
    std::unique_ptr<Expr> operand;
};

struct BinaryExpr : Expr {
    BinaryExpr(SourceLocation loc, std::string expressionOp, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
        : Expr(loc)
        , op(std::move(expressionOp))
        , left(std::move(lhs))
        , right(std::move(rhs)) {
    }

    std::string op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
};

struct Stmt {
    explicit Stmt(SourceLocation loc)
        : location(loc) {
    }

    virtual ~Stmt() = default;

    SourceLocation location;
};

struct VarDeclStmt : Stmt {
    VarDeclStmt(SourceLocation loc, std::vector<std::string> variableNames, ValueType valueType)
        : Stmt(loc)
        , names(std::move(variableNames))
        , type(valueType) {
    }

    std::vector<std::string> names;
    ValueType type;
};

struct AssignStmt : Stmt {
    AssignStmt(SourceLocation loc, std::string variableName, std::unique_ptr<Expr> expressionValue)
        : Stmt(loc)
        , name(std::move(variableName))
        , value(std::move(expressionValue)) {
    }

    std::string name;
    std::unique_ptr<Expr> value;
};

struct WriteStmt : Stmt {
    WriteStmt(SourceLocation loc, std::unique_ptr<Expr> expression)
        : Stmt(loc)
        , expr(std::move(expression)) {
    }

    std::unique_ptr<Expr> expr;
};

struct ReadStmt : Stmt {
    ReadStmt(SourceLocation loc, std::string variableName)
        : Stmt(loc)
        , name(std::move(variableName)) {
    }

    std::string name;
    ValueType targetType {ValueType::Unknown};
};

struct IfStmt : Stmt {
    explicit IfStmt(SourceLocation loc)
        : Stmt(loc) {
    }

    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> thenBranch;
    std::vector<std::unique_ptr<Stmt>> elseBranch;
};

struct WhileStmt : Stmt {
    explicit WhileStmt(SourceLocation loc)
        : Stmt(loc) {
    }

    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> body;
};

struct Program {
    Program(SourceLocation loc, std::string programName)
        : location(loc)
        , name(std::move(programName)) {
    }

    SourceLocation location;
    std::string name;
    std::vector<std::unique_ptr<Stmt>> declarations;
    std::vector<std::unique_ptr<Stmt>> statements;
};

std::string dumpAst(const Program& program);

} // namespace portugol
