// Lightweight test runner for the Spike Compiler.
// Real unit tests (lexer, parser, codegen, etc.) will grow here over time.
// Each test prints a line and increments a failure counter on assertion error;
// the runner ends with "all tests passed" iff zero failures occurred, which
// is what CTest matches against.

#include "spike/file_reader.hpp"
#include "spike/token.hpp"

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

// ---- file_reader -----------------------------------------------------------

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

// ---- token -----------------------------------------------------------------

void test_token_type_to_string_known_values() {
    SPIKE_EXPECT(spike::token_type_to_string(spike::TokenType::NUMBER) == "NUMBER");
    SPIKE_EXPECT(spike::token_type_to_string(spike::TokenType::ALGORITMO) == "ALGORITMO");
    SPIKE_EXPECT(spike::token_type_to_string(spike::TokenType::FIM_ENQUANTO) == "FIM_ENQUANTO");
    SPIKE_EXPECT(spike::token_type_to_string(spike::TokenType::ARROW) == "ARROW");
    SPIKE_EXPECT(spike::token_type_to_string(spike::TokenType::END_OF_FILE) == "END_OF_FILE");
    SPIKE_EXPECT(spike::token_type_to_string(spike::TokenType::UNKNOWN) == "UNKNOWN");
}

void test_token_default_construction() {
    spike::Token t{spike::TokenType::IDENTIFIER, "contador", 3, 7};
    SPIKE_EXPECT(t.type == spike::TokenType::IDENTIFIER);
    SPIKE_EXPECT(t.lexeme == "contador");
    SPIKE_EXPECT(t.line == 3);
    SPIKE_EXPECT(t.col == 7);
}

} // namespace

int main() {
    test_read_file_missing();
    test_read_file_ok();
    test_read_file_wrong_extension_still_ok();
    test_token_type_to_string_known_values();
    test_token_default_construction();

    if (g_failures == 0) {
        std::cout << "all tests passed" << std::endl;
        return 0;
    }
    std::cerr << g_failures << " test(s) failed" << std::endl;
    return 1;
}
