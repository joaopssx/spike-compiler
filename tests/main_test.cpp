// Lightweight test runner for the Spike Compiler.
// Real unit tests (lexer, parser, codegen, etc.) will grow here over time.
// Each test prints a line and increments a failure counter on assertion error;
// the runner ends with "all tests passed" iff zero failures occurred, which
// is what CTest matches against.

#include "spike/file_reader.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

int g_failures = 0;

#define SPIKE_EXPECT(cond)                                                     \
    do {                                                                       \
        if (!(cond)) {                                                         \
            ++g_failures;                                                      \
            std::cerr << "FAIL: " << __FILE__ << ":" << __LINE__               \
                      << ": " #cond << std::endl;                              \
        }                                                                      \
    } while (0)

void test_read_file_missing() {
    spike::ReadResult r = spike::read_file("definitely_does_not_exist_xyz.por");
    SPIKE_EXPECT(r.ok == false);
    SPIKE_EXPECT(r.error.find("Arquivo não encontrado") != std::string::npos);
}

void test_read_file_ok() {
    const std::string path = "test_temp.por";
    const std::string payload = "algoritmo \"teste\"\n";

    {
        std::ofstream out(path, std::ios::binary);
        out << payload;
    }

    spike::ReadResult r = spike::read_file(path);
    SPIKE_EXPECT(r.ok == true);
    SPIKE_EXPECT(r.content == payload);
    SPIKE_EXPECT(r.error.empty());

    std::error_code ec;
    std::filesystem::remove(path, ec);
}

void test_read_file_wrong_extension_still_ok() {
    const std::string path = "test_temp.txt";
    const std::string payload = "qualquer coisa";

    {
        std::ofstream out(path, std::ios::binary);
        out << payload;
    }

    spike::ReadResult r = spike::read_file(path);
    SPIKE_EXPECT(r.ok == true);
    SPIKE_EXPECT(r.content == payload);
    SPIKE_EXPECT(!r.error.empty()); // warning about missing .por extension

    std::error_code ec;
    std::filesystem::remove(path, ec);
}

} // namespace

int main() {
    test_read_file_missing();
    test_read_file_ok();
    test_read_file_wrong_extension_still_ok();

    if (g_failures == 0) {
        std::cout << "all tests passed" << std::endl;
        return 0;
    }
    std::cerr << g_failures << " test(s) failed" << std::endl;
    return 1;
}
