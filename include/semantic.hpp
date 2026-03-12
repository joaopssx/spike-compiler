#pragma once

#include <string>
#include <unordered_map>

#include "ast.hpp"

namespace portugol {

class SemanticAnalyzer {
public:
    void analyze(Program& program);

private:
    struct Symbol {
        ValueType type;
        SourceLocation location;
    };

    void declareVariables(const VarDeclStmt& declaration);
    void analyzeStmt(Stmt& stmt);
    ValueType analyzeExpr(Expr& expr);
    const Symbol& requireSymbol(const std::string& name, const SourceLocation& location) const;

    static bool isNumeric(ValueType type);
    static bool isConditionType(ValueType type);
    static bool canAssign(ValueType target, ValueType source);

    std::unordered_map<std::string, Symbol> symbols_;
};

} // namespace portugol
