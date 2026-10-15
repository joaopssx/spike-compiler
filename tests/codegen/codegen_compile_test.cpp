#include "codegen/codegen_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

namespace {

using spike_tests::generate_from_source;

#ifdef SPIKE_TEST_C_COMPILER

int run_shell(std::string command) {
#ifdef _WIN32
    command = "\"" + command + "\"";
#endif
    return std::system(command.c_str());
}

void test_generated_c_compiles_without_warnings() {
    const auto gen = generate_from_source(
        "algoritmo \"compila\"\n"
        "var\n  a, i : inteiro\n  r : real\n  t, u : caractere\n  b, usado : logico\n"
        "inicio\n"
        "  a <- 10\n  i <- a * 2 - -3\n  r <- a / 4 + 0.5\n  r <- -r\n"
        "  t <- \"Olá, \\\"mundo\\\"\\n\"\n  u <- t + \"!\"\n"
        "  b <- (t < u) e nao (a = i) ou (r >= 2)\n  usado <- b = falso\n"
        "fimalgoritmo\n",
        "compila.por");
    SPIKE_EXPECT(gen.result.ok);
    if (!gen.result.ok) return;

    const std::string base = std::string(SPIKE_BINARY_DIR) + "/codegen_compile_test";
    {
        std::ofstream file(base + ".c", std::ios::binary);
        file << gen.result.c_source;
    }
    const std::string exe = base + ".exe";
    const std::string compile = std::string("\"") + SPIKE_TEST_C_COMPILER +
                                "\" -std=c99 -Wall -Wextra -pedantic -Werror \"" + base +
                                ".c\" -o \"" + exe + "\"";
    const int compiled = run_shell(compile);
    if (compiled != 0) std::cerr << "  compile command failed: " << compile << "\n";
    SPIKE_EXPECT(compiled == 0);
    if (compiled != 0) return;
    SPIKE_EXPECT(run_shell("\"" + exe + "\"") == 0);
}

#else

void test_generated_c_compiles_without_warnings() {}

#endif

} // namespace

namespace spike_tests {

void run_codegen_compile_tests() {
    test_generated_c_compiles_without_warnings();
}

} // namespace spike_tests
