#pragma once

#include <string>

#include "ast.hpp"

namespace portugol {

std::string astToGraphviz(const Program& program);

} // namespace portugol
