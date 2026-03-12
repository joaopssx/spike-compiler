#include "program_stats.hpp"

#include <sstream>

namespace portugol {

namespace {

void countExpr(const Expr& expr, ProgramStats& stats);

void countStmt(const Stmt& stmt, ProgramStats& stats) {
    ++stats.statementCount;

    if (const auto* decl = dynamic_cast<const VarDeclStmt*>(&stmt)) {
        ++stats.declarationCount;
        stats.variableCount += decl->names.size();
        return;
    }

    if (const auto* assign = dynamic_cast<const AssignStmt*>(&stmt)) {
        ++stats.assignmentCount;
        countExpr(*assign->value, stats);
        return;
    }

    if (const auto* write = dynamic_cast<const WriteStmt*>(&stmt)) {
        ++stats.writeCount;
        countExpr(*write->expr, stats);
        return;
    }

    if (dynamic_cast<const ReadStmt*>(&stmt) != nullptr) {
        ++stats.readCount;
        return;
    }

    if (const auto* ifStmt = dynamic_cast<const IfStmt*>(&stmt)) {
        ++stats.ifCount;
        countExpr(*ifStmt->condition, stats);
        for (const auto& child : ifStmt->thenBranch) {
            countStmt(*child, stats);
        }
        for (const auto& child : ifStmt->elseBranch) {
            countStmt(*child, stats);
        }
        return;
    }

    if (const auto* whileStmt = dynamic_cast<const WhileStmt*>(&stmt)) {
        ++stats.whileCount;
        countExpr(*whileStmt->condition, stats);
        for (const auto& child : whileStmt->body) {
            countStmt(*child, stats);
        }
        return;
    }
}

void countExpr(const Expr& expr, ProgramStats& stats) {
    ++stats.expressionCount;

    if (dynamic_cast<const IntExpr*>(&expr) != nullptr) {
        ++stats.intLiteralCount;
        return;
    }

    if (dynamic_cast<const RealExpr*>(&expr) != nullptr) {
        ++stats.realLiteralCount;
        return;
    }

    if (dynamic_cast<const VariableExpr*>(&expr) != nullptr) {
        ++stats.variableExpressionCount;
        return;
    }

    if (const auto* unaryExpr = dynamic_cast<const UnaryExpr*>(&expr)) {
        ++stats.unaryExpressionCount;
        countExpr(*unaryExpr->operand, stats);
        return;
    }

    if (const auto* binaryExpr = dynamic_cast<const BinaryExpr*>(&expr)) {
        ++stats.binaryExpressionCount;
        countExpr(*binaryExpr->left, stats);
        countExpr(*binaryExpr->right, stats);
    }
}

} // namespace

ProgramStats collectProgramStats(const Program& program, std::size_t lineCount, std::size_t tokenCount) {
    ProgramStats stats;
    stats.lineCount = lineCount;
    stats.tokenCount = tokenCount;

    for (const auto& declaration : program.declarations) {
        countStmt(*declaration, stats);
    }
    for (const auto& statement : program.statements) {
        countStmt(*statement, stats);
    }

    return stats;
}

std::string renderProgramStats(const ProgramStats& stats) {
    std::ostringstream out;
    out << "Program Statistics\n\n";
    out << "Lines: " << stats.lineCount << "\n";
    out << "Tokens: " << stats.tokenCount << "\n";
    out << "Declarations: " << stats.declarationCount << "\n";
    out << "Variables: " << stats.variableCount << "\n";
    out << "Statements: " << stats.statementCount << "\n";
    out << "Assignments: " << stats.assignmentCount << "\n";
    out << "Writes: " << stats.writeCount << "\n";
    out << "Reads: " << stats.readCount << "\n";
    out << "If statements: " << stats.ifCount << "\n";
    out << "Loops: " << stats.whileCount << "\n";
    out << "Expressions: " << stats.expressionCount << "\n";
    out << "Binary expressions: " << stats.binaryExpressionCount << "\n";
    out << "Unary expressions: " << stats.unaryExpressionCount << "\n";
    return out.str();
}

std::string renderProgramStatsJson(const ProgramStats& stats) {
    std::ostringstream out;
    out << "{\n";
    out << "  \"lines\": " << stats.lineCount << ",\n";
    out << "  \"tokens\": " << stats.tokenCount << ",\n";
    out << "  \"declarations\": " << stats.declarationCount << ",\n";
    out << "  \"variables\": " << stats.variableCount << ",\n";
    out << "  \"statements\": " << stats.statementCount << ",\n";
    out << "  \"assignments\": " << stats.assignmentCount << ",\n";
    out << "  \"writes\": " << stats.writeCount << ",\n";
    out << "  \"reads\": " << stats.readCount << ",\n";
    out << "  \"ifs\": " << stats.ifCount << ",\n";
    out << "  \"loops\": " << stats.whileCount << ",\n";
    out << "  \"expressions\": " << stats.expressionCount << ",\n";
    out << "  \"binaryExpressions\": " << stats.binaryExpressionCount << ",\n";
    out << "  \"unaryExpressions\": " << stats.unaryExpressionCount << ",\n";
    out << "  \"intLiterals\": " << stats.intLiteralCount << ",\n";
    out << "  \"realLiterals\": " << stats.realLiteralCount << ",\n";
    out << "  \"variableExpressions\": " << stats.variableExpressionCount << "\n";
    out << "}\n";
    return out.str();
}

} // namespace portugol
