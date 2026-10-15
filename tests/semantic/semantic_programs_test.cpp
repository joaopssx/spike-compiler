#include "semantic/semantic_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/file_reader.hpp"

#include <cctype>
#include <filesystem>
#include <iostream>
#include <string>

namespace {

using spike_tests::analyze_source;
using spike_tests::count_occurrences;

std::string expected_code(const std::string& stem) {
    std::string code = stem.substr(0, 4);
    for (char& c : code) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return code;
}

void test_semantic_invalid_programs_emit_their_code() {
    const std::filesystem::path dir =
        std::filesystem::path(SPIKE_SOURCE_DIR) / "tests" / "programs" / "semantic_invalid";
    int checked = 0;
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.path().extension() != ".por") continue;
        const spike::ReadResult file = spike::read_file(entry.path().string());
        SPIKE_EXPECT(file.ok);
        const std::string code = expected_code(entry.path().stem().string());
        const auto r = analyze_source(file.content);
        const bool ok = !r.parse_error && count_occurrences(r.diag_output, "erro[") == 1 &&
                        r.diag_output.find("[" + code + "]") != std::string::npos;
        if (!ok) std::cerr << "  program: " << entry.path().filename().string() << "\n";
        SPIKE_EXPECT(ok);
        ++checked;
    }
    SPIKE_EXPECT(checked >= 12);
}

} // namespace

namespace spike_tests {

void run_semantic_program_tests() {
    test_semantic_invalid_programs_emit_their_code();
}

} // namespace spike_tests
