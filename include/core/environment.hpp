#pragma once

#include <string>
#include <vector>

namespace portugol::core {

struct EnvironmentCheck {
    std::string label;
    bool ok {false};
    std::string detail;
};

class Environment {
public:
    static std::string platformName();
    static std::vector<EnvironmentCheck> doctorChecks(const std::string& cCompiler);
};

} // namespace portugol::core
