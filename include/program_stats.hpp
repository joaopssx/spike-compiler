#pragma once

#include <cstddef>
#include <string>

#include "ast.hpp"

namespace portugol {

struct ProgramStats {
    std::size_t lineCount {0};
    std::size_t tokenCount {0};
    std::size_t declarationCount {0};
    std::size_t variableCount {0};
    std::size_t statementCount {0};
    std::size_t assignmentCount {0};
    std::size_t writeCount {0};
    std::size_t readCount {0};
    std::size_t ifCount {0};
    std::size_t whileCount {0};
    std::size_t expressionCount {0};
    std::size_t binaryExpressionCount {0};
    std::size_t unaryExpressionCount {0};
    std::size_t intLiteralCount {0};
    std::size_t realLiteralCount {0};
    std::size_t variableExpressionCount {0};
};

ProgramStats collectProgramStats(const Program& program, std::size_t lineCount, std::size_t tokenCount);
std::string renderProgramStats(const ProgramStats& stats);
std::string renderProgramStatsJson(const ProgramStats& stats);

} // namespace portugol
