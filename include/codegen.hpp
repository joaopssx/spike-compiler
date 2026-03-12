#pragma once

#include <cstddef>
#include <string>

#include "ast.hpp"

namespace portugol {

class CodeGenerator {
public:
    std::string generate(const Program& program) const;

private:
    void emitStmt(const Stmt& stmt, std::size_t indentLevel, std::string& out) const;
    std::string emitExpr(const Expr& expr) const;
    std::string emitIndent(std::size_t indentLevel) const;
    std::string cType(ValueType type) const;
    std::string printfFormat(ValueType type) const;
    std::string scanfFormat(ValueType type) const;
};

} // namespace portugol
