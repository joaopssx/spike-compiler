#include "pass_manager.hpp"

#include <cmath>
#include <memory>

namespace portugol {

namespace {

bool isNumericLiteral(const Expr& expr) {
    return dynamic_cast<const IntExpr*>(&expr) != nullptr || dynamic_cast<const RealExpr*>(&expr) != nullptr;
}

double asDouble(const Expr& expr) {
    if (const auto* intExpr = dynamic_cast<const IntExpr*>(&expr)) {
        return static_cast<double>(intExpr->value);
    }
    if (const auto* realExpr = dynamic_cast<const RealExpr*>(&expr)) {
        return realExpr->value;
    }
    return 0.0;
}

bool isIntegral(const Expr& expr) {
    return dynamic_cast<const IntExpr*>(&expr) != nullptr;
}

std::unique_ptr<Expr> makeNumericLiteral(const Expr& original, double value, bool asInteger, ValueType inferredType) {
    if (asInteger) {
        auto folded = std::make_unique<IntExpr>(original.location, static_cast<int>(value));
        folded->inferredType = inferredType;
        return folded;
    }

    auto folded = std::make_unique<RealExpr>(original.location, value);
    folded->inferredType = inferredType;
    return folded;
}

std::size_t foldExpr(std::unique_ptr<Expr>& expr) {
    std::size_t folds = 0;

    if (auto* unaryExpr = dynamic_cast<UnaryExpr*>(expr.get())) {
        folds += foldExpr(unaryExpr->operand);
        if (unaryExpr->op == "-" && isNumericLiteral(*unaryExpr->operand)) {
            const double value = -asDouble(*unaryExpr->operand);
            const bool asInteger = isIntegral(*unaryExpr->operand);
            const ValueType inferred = unaryExpr->inferredType == ValueType::Unknown
                ? (asInteger ? ValueType::Inteiro : ValueType::Real)
                : unaryExpr->inferredType;
            expr = makeNumericLiteral(*unaryExpr, value, asInteger, inferred);
            ++folds;
        }
        return folds;
    }

    if (auto* binaryExpr = dynamic_cast<BinaryExpr*>(expr.get())) {
        folds += foldExpr(binaryExpr->left);
        folds += foldExpr(binaryExpr->right);

        if (isNumericLiteral(*binaryExpr->left) && isNumericLiteral(*binaryExpr->right)) {
            const double left = asDouble(*binaryExpr->left);
            const double right = asDouble(*binaryExpr->right);
            const bool bothIntegral = isIntegral(*binaryExpr->left) && isIntegral(*binaryExpr->right);

            if (binaryExpr->op == "+") {
                expr = makeNumericLiteral(*binaryExpr, left + right, bothIntegral, bothIntegral ? ValueType::Inteiro : ValueType::Real);
                ++folds;
            } else if (binaryExpr->op == "-") {
                expr = makeNumericLiteral(*binaryExpr, left - right, bothIntegral, bothIntegral ? ValueType::Inteiro : ValueType::Real);
                ++folds;
            } else if (binaryExpr->op == "*") {
                expr = makeNumericLiteral(*binaryExpr, left * right, bothIntegral, bothIntegral ? ValueType::Inteiro : ValueType::Real);
                ++folds;
            } else if (binaryExpr->op == "/") {
                expr = makeNumericLiteral(*binaryExpr, left / right, false, ValueType::Real);
                ++folds;
            } else if (binaryExpr->op == "=") {
                expr = makeNumericLiteral(*binaryExpr, left == right ? 1.0 : 0.0, true, ValueType::Logico);
                ++folds;
            } else if (binaryExpr->op == "<>") {
                expr = makeNumericLiteral(*binaryExpr, left != right ? 1.0 : 0.0, true, ValueType::Logico);
                ++folds;
            } else if (binaryExpr->op == "<") {
                expr = makeNumericLiteral(*binaryExpr, left < right ? 1.0 : 0.0, true, ValueType::Logico);
                ++folds;
            } else if (binaryExpr->op == "<=") {
                expr = makeNumericLiteral(*binaryExpr, left <= right ? 1.0 : 0.0, true, ValueType::Logico);
                ++folds;
            } else if (binaryExpr->op == ">") {
                expr = makeNumericLiteral(*binaryExpr, left > right ? 1.0 : 0.0, true, ValueType::Logico);
                ++folds;
            } else if (binaryExpr->op == ">=") {
                expr = makeNumericLiteral(*binaryExpr, left >= right ? 1.0 : 0.0, true, ValueType::Logico);
                ++folds;
            }
        }
    }

    return folds;
}

std::size_t foldStmt(Stmt& stmt) {
    std::size_t folds = 0;

    if (auto* assign = dynamic_cast<AssignStmt*>(&stmt)) {
        folds += foldExpr(assign->value);
        return folds;
    }
    if (auto* write = dynamic_cast<WriteStmt*>(&stmt)) {
        folds += foldExpr(write->expr);
        return folds;
    }
    if (auto* ifStmt = dynamic_cast<IfStmt*>(&stmt)) {
        folds += foldExpr(ifStmt->condition);
        for (auto& child : ifStmt->thenBranch) {
            folds += foldStmt(*child);
        }
        for (auto& child : ifStmt->elseBranch) {
            folds += foldStmt(*child);
        }
        return folds;
    }
    if (auto* whileStmt = dynamic_cast<WhileStmt*>(&stmt)) {
        folds += foldExpr(whileStmt->condition);
        for (auto& child : whileStmt->body) {
            folds += foldStmt(*child);
        }
    }

    return folds;
}

} // namespace

PassReport runOptimizationPasses(Program& program) {
    PassReport report;
    for (auto& declaration : program.declarations) {
        report.constantFolds += foldStmt(*declaration);
    }
    for (auto& statement : program.statements) {
        report.constantFolds += foldStmt(*statement);
    }
    return report;
}

} // namespace portugol
