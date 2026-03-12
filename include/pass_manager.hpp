#pragma once

#include <cstddef>

#include "ast.hpp"

namespace portugol {

struct PassReport {
    std::size_t constantFolds {0};
};

PassReport runOptimizationPasses(Program& program);

} // namespace portugol
