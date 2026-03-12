#include "semantic.hpp"

#include <utility>

#include "utils.hpp"

namespace portugol {

namespace {

bool isArithmeticOperator(const std::string& op) {
    return op == "+" || op == "-" || op == "*" || op == "/";
}

bool isRelationalOperator(const std::string& op) {
    return op == "=" || op == "<>" || op == "<" || op == "<=" || op == ">" || op == ">=";
}

} // namespace

void SemanticAnalyzer::analyze(Program& program) {
    symbols_.clear();

    for (const auto& declaration : program.declarations) {
        const auto* varDecl = dynamic_cast<const VarDeclStmt*>(declaration.get());
        if (varDecl == nullptr) {
            throw CompilerError("semantico", "declaracao invalida no bloco 'var'", declaration->location);
        }
        declareVariables(*varDecl);
    }

    for (const auto& stmt : program.statements) {
        analyzeStmt(*stmt);
    }
}

void SemanticAnalyzer::declareVariables(const VarDeclStmt& declaration) {
    for (const auto& name : declaration.names) {
        if (symbols_.find(name) != symbols_.end()) {
            throw CompilerError("semantico", "variavel '" + name + "' ja foi declarada", declaration.location);
        }

        symbols_.insert({name, Symbol {declaration.type, declaration.location}});
    }
}

void SemanticAnalyzer::analyzeStmt(Stmt& stmt) {
    if (auto* assign = dynamic_cast<AssignStmt*>(&stmt)) {
        const auto& symbol = requireSymbol(assign->name, assign->location);
        const ValueType valueType = analyzeExpr(*assign->value);
        if (!canAssign(symbol.type, valueType)) {
            throw CompilerError("semantico", "nao pode atribuir um valor do tipo '" + std::string(valueTypeName(valueType)) + "' para a variavel '" + assign->name + "' do tipo '" + std::string(valueTypeName(symbol.type)) + "'", assign->location);
        }
        return;
    }

    if (auto* write = dynamic_cast<WriteStmt*>(&stmt)) {
        analyzeExpr(*write->expr);
        return;
    }

    if (auto* read = dynamic_cast<ReadStmt*>(&stmt)) {
        read->targetType = requireSymbol(read->name, read->location).type;
        return;
    }

    if (auto* ifStmt = dynamic_cast<IfStmt*>(&stmt)) {
        const ValueType conditionType = analyzeExpr(*ifStmt->condition);
        if (!isConditionType(conditionType)) {
            throw CompilerError("semantico", "a condicao do 'se' precisa ser numerica ou relacional", ifStmt->condition->location);
        }
        for (const auto& child : ifStmt->thenBranch) {
            analyzeStmt(*child);
        }
        for (const auto& child : ifStmt->elseBranch) {
            analyzeStmt(*child);
        }
        return;
    }

    if (auto* whileStmt = dynamic_cast<WhileStmt*>(&stmt)) {
        const ValueType conditionType = analyzeExpr(*whileStmt->condition);
        if (!isConditionType(conditionType)) {
            throw CompilerError("semantico", "a condicao do 'enquanto' precisa ser numerica ou relacional", whileStmt->condition->location);
        }
        for (const auto& child : whileStmt->body) {
            analyzeStmt(*child);
        }
        return;
    }
}

ValueType SemanticAnalyzer::analyzeExpr(Expr& expr) {
    if (dynamic_cast<IntExpr*>(&expr) != nullptr) {
        expr.inferredType = ValueType::Inteiro;
        return expr.inferredType;
    }

    if (dynamic_cast<RealExpr*>(&expr) != nullptr) {
        expr.inferredType = ValueType::Real;
        return expr.inferredType;
    }

    if (auto* variable = dynamic_cast<VariableExpr*>(&expr)) {
        expr.inferredType = requireSymbol(variable->name, variable->location).type;
        return expr.inferredType;
    }

    if (auto* unary = dynamic_cast<UnaryExpr*>(&expr)) {
        const ValueType operandType = analyzeExpr(*unary->operand);
        if (unary->op == "-" && !isNumeric(operandType)) {
            throw CompilerError("semantico", "o operador '-' so aceita operandos numericos", unary->location);
        }
        expr.inferredType = operandType;
        return expr.inferredType;
    }

    if (auto* binary = dynamic_cast<BinaryExpr*>(&expr)) {
        const ValueType leftType = analyzeExpr(*binary->left);
        const ValueType rightType = analyzeExpr(*binary->right);

        if (isArithmeticOperator(binary->op)) {
            if (!isNumeric(leftType) || !isNumeric(rightType)) {
                throw CompilerError("semantico", "operacao aritmetica exige operandos numericos", binary->location);
            }

            expr.inferredType = binary->op == "/" || leftType == ValueType::Real || rightType == ValueType::Real
                ? ValueType::Real
                : ValueType::Inteiro;
            return expr.inferredType;
        }

        if (isRelationalOperator(binary->op)) {
            if (!isNumeric(leftType) || !isNumeric(rightType)) {
                throw CompilerError("semantico", "operacao relacional exige operandos numericos", binary->location);
            }

            expr.inferredType = ValueType::Logico;
            return expr.inferredType;
        }
    }

    throw CompilerError("semantico", "expressao desconhecida", expr.location);
}

const SemanticAnalyzer::Symbol& SemanticAnalyzer::requireSymbol(const std::string& name, const SourceLocation& location) const {
    const auto it = symbols_.find(name);
    if (it == symbols_.end()) {
        throw CompilerError("semantico", "variavel '" + name + "' nao foi declarada", location);
    }

    return it->second;
}

bool SemanticAnalyzer::isNumeric(ValueType type) {
    return type == ValueType::Inteiro || type == ValueType::Real;
}

bool SemanticAnalyzer::isConditionType(ValueType type) {
    return isNumeric(type) || type == ValueType::Logico;
}

bool SemanticAnalyzer::canAssign(ValueType target, ValueType source) {
    if (target == source) {
        return true;
    }

    return target == ValueType::Real && source == ValueType::Inteiro;
}

} // namespace portugol
