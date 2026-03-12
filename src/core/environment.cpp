#include "core/environment.hpp"

#include <cstdlib>

namespace portugol::core {

namespace {

bool commandAvailable(const std::string& command) {
#ifdef _WIN32
    const std::string shellCommand = "where " + command + " >nul 2>nul";
#else
    const std::string shellCommand = "command -v " + command + " >/dev/null 2>&1";
#endif
    return std::system(shellCommand.c_str()) == 0;
}

} // namespace

std::string Environment::platformName() {
#ifdef _WIN32
    return "windows";
#elif __APPLE__
    return "macos";
#elif __linux__
    return "linux";
#else
    return "unknown";
#endif
}

std::vector<EnvironmentCheck> Environment::doctorChecks(const std::string& cCompiler) {
    std::vector<EnvironmentCheck> checks;
    checks.push_back(EnvironmentCheck {"platform", true, platformName()});
    checks.push_back(EnvironmentCheck {"cmake", commandAvailable("cmake"), commandAvailable("cmake") ? "cmake encontrado" : "cmake nao encontrado"});
    checks.push_back(EnvironmentCheck {"c++ compiler", commandAvailable("c++") || commandAvailable("g++") || commandAvailable("clang++"), "busca por c++, g++ ou clang++"});
    checks.push_back(EnvironmentCheck {"c compiler", commandAvailable(cCompiler), cCompiler + (commandAvailable(cCompiler) ? " encontrado" : " nao encontrado")});
    return checks;
}

} // namespace portugol::core
