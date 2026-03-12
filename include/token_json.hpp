#pragma once

#include <string>
#include <vector>

#include "token.hpp"

namespace portugol {

std::string tokensToJson(const std::vector<Token>& tokens);

} // namespace portugol
