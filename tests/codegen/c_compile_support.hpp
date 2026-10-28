#ifndef SPIKE_TESTS_C_COMPILE_SUPPORT_HPP
#define SPIKE_TESTS_C_COMPILE_SUPPORT_HPP

#ifdef SPIKE_TEST_C_COMPILER

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace spike_tests {

inline int run_shell(std::string command) {
#ifdef _WIN32
    command = "\"" + command + "\"";
#endif
    return std::system(command.c_str());
}

inline std::string read_all(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::string without_cr;
    for (char c : text) {
        if (c != '\r') without_cr += c;
    }
    return without_cr;
}

inline bool compile_c(const std::string& c_source, const std::string& base) {
    {
        std::ofstream file(base + ".c", std::ios::binary);
        file << c_source;
    }
    const std::string compile = std::string("\"") + SPIKE_TEST_C_COMPILER +
                                "\" -std=c99 -Wall -Wextra -pedantic -Werror \"" + base +
                                ".c\" -o \"" + base + ".exe\" -lm";
    const int status = run_shell(compile);
    if (status != 0) std::cerr << "  compile command failed: " << compile << "\n";
    return status == 0;
}

inline int run_to_file(const std::string& base) {
    return run_shell("\"" + base + ".exe\" > \"" + base + ".out\" 2> \"" + base + ".err\"");
}

} // namespace spike_tests

#endif

#endif // SPIKE_TESTS_C_COMPILE_SUPPORT_HPP
