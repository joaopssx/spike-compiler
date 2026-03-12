#pragma once

#include "ast.hpp"
#include "ir.hpp"

namespace portugol {

class IRBuilder {
public:
    IRProgram build(const Program& program);

private:
    std::string buildExpr(const Expr& expr, IRProgram& ir);
    void buildStmt(const Stmt& stmt, IRProgram& ir);
    std::string newTemp();
    std::string newLabel(const std::string& prefix);

    std::size_t tempCounter_ {0};
    std::size_t labelCounter_ {0};
};

} // namespace portugol
