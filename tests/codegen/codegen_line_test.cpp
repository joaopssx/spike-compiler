#include "codegen/c_compile_support.hpp"
#include "codegen/codegen_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/cc_driver.hpp"
#include "spike/file_reader.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

using spike_tests::contains;
using spike_tests::generate_from_source;

const char* kProgram = "algoritmo \"linhas\"\n"
                       "var\n  a : inteiro\n"
                       "procedimento mostra(n: inteiro)\n"
                       "inicio\n"
                       "  escreval(\"n = \", n)\n"
                       "fimprocedimento\n"
                       "inicio\n"
                       "  a <- 2\n"
                       "  se a > 1 entao\n"
                       "    mostra(a)\n"
                       "  fimse\n"
                       "fimalgoritmo\n";

std::vector<std::string> lines_of(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) lines.push_back(line);
    return lines;
}

void test_source_lines_are_embedded_once() {
    const auto gen = generate_from_source("algoritmo \"t\"\r\ninicio\r\n  escreval(\"a\\\\b\")\r\n"
                                          "fimalgoritmo\r\n",
                                          "ola.por");
    SPIKE_EXPECT(gen.result.ok);
    const std::string& c = gen.result.c_source;
    SPIKE_EXPECT(contains(c, "static const char* const spike_source_name = \"ola.por\";\n"
                             "static const char* const spike_source_lines[] = {\n"
                             "    \"algoritmo \\\"t\\\"\",\n"
                             "    \"inicio\",\n"
                             "    \"  escreval(\\\"a\\\\\\\\b\\\")\",\n"
                             "    \"fimalgoritmo\",\n"
                             "};\n"));
    SPIKE_EXPECT(contains(c, "    spike_set_source(spike_source_name, spike_source_lines, 4);\n"));
    SPIKE_EXPECT(c.find("spike_source_lines[] = {") < c.find("#ifndef SPIKE_RUNTIME_CORE_H"));
    SPIKE_EXPECT(c.find('\r') == std::string::npos);
}

void test_no_line_directives_by_default() {
    const auto gen = generate_from_source(kProgram, "linhas.por");
    SPIKE_EXPECT(gen.result.ok);
    SPIKE_EXPECT(!contains(gen.result.c_source, "#line"));
}

void test_line_directive_before_each_statement() {
    const auto gen = generate_from_source(kProgram, "linhas.por", true);
    SPIKE_EXPECT(gen.result.ok);
    const std::string& c = gen.result.c_source;
    SPIKE_EXPECT(contains(c, "#line 6 \"linhas.por\"\n    spike_write_str(\"n = \");\n"));
    SPIKE_EXPECT(contains(c, "#line 9 \"linhas.por\"\n    v_a = 2LL;\n"));
    SPIKE_EXPECT(contains(c, "#line 10 \"linhas.por\"\n    if (v_a > 1LL) {\n"));
    SPIKE_EXPECT(contains(c, "#line 11 \"linhas.por\"\n        p_mostra(11, v_a);\n"));
}

void test_line_directive_returns_to_generated_file() {
    const auto gen = generate_from_source(kProgram, "linhas.por", true);
    const std::vector<std::string> lines = lines_of(gen.result.c_source);
    const std::string prefix = "#line ";
    const std::string suffix = " \"programa.c\"";
    int returns = 0;
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const std::string& line = lines[i];
        if (line.rfind(prefix, 0) != 0 || line.size() <= suffix.size() ||
            line.compare(line.size() - suffix.size(), suffix.size(), suffix) != 0) {
            continue;
        }
        ++returns;
        const int number = std::atoi(line.c_str() + prefix.size());
        SPIKE_EXPECT(number == static_cast<int>(i) + 2);
    }
    SPIKE_EXPECT(returns == 2);
    const std::string& c = gen.result.c_source;
    SPIKE_EXPECT(c.find("\"programa.c\"\n    spike_leave();\n") != std::string::npos);
    SPIKE_EXPECT(c.find("\"programa.c\"\n    spike_flush();\n") != std::string::npos);
}

#ifdef SPIKE_TEST_C_COMPILER

using spike_tests::compile_c;
using spike_tests::read_all;
using spike_tests::run_to_file;

std::string run_generated(const std::string& source, const std::string& name, bool lines,
                          const std::string& base) {
    const auto gen = generate_from_source(source, name, lines);
    SPIKE_EXPECT(gen.result.ok);
    if (!gen.result.ok || !compile_c(gen.result.c_source, base)) return "(did not compile)";
    SPIKE_EXPECT(run_to_file(base) == 0);
    SPIKE_EXPECT(read_all(base + ".err").empty());
    return read_all(base + ".out");
}

void test_line_directives_do_not_change_the_program() {
    for (const char* example : {"bubble_sort", "fibonacci", "procedures"}) {
        const auto file =
            spike::read_file(std::string(SPIKE_SOURCE_DIR) + "/examples/" + example + ".por");
        SPIKE_EXPECT(file.ok);
        const std::string base = std::string(SPIKE_BINARY_DIR) + "/codegen_line_" + example;
        const std::string name = std::string(example) + ".por";
        const std::string plain = run_generated(file.content, name, false, base + "_plain");
        const std::string marked = run_generated(file.content, name, true, base + "_marked");
        if (plain != marked) std::cerr << "  " << example << " differs with #line\n";
        SPIKE_EXPECT(!plain.empty());
        SPIKE_EXPECT(plain == marked);
    }
}

void test_compiler_error_points_to_por_line() {
    auto gen = generate_from_source(kProgram, "linhas.por", true);
    SPIKE_EXPECT(gen.result.ok);
    std::string& c = gen.result.c_source;
    const std::string marker = "#line 9 \"linhas.por\"\n";
    const std::size_t at = c.find(marker);
    SPIKE_EXPECT(at != std::string::npos);
    if (at == std::string::npos) return;
    c.insert(at + marker.size(), "    int broken = ;\n");

    const std::string base = std::string(SPIKE_BINARY_DIR) + "/codegen_line_broken";
    {
        std::ofstream file(base + ".c", std::ios::binary);
        file << c;
    }
    const spike::CompilerInfo compiler{SPIKE_TEST_C_COMPILER,
                                       spike::compiler_kind(SPIKE_TEST_C_COMPILER)};
    const spike::CompileResult result =
        spike::compile_c(compiler, spike::CompileRequest{base + ".c", base + ".exe", false, std::nullopt});
    SPIKE_EXPECT(!result.ok);
    SPIKE_EXPECT(spike::compiler_error_line(result.compiler_output, "linhas.por") == 9);
    spike::DriverError error = result.error;
    spike::point_to_source(error, result.compiler_output, "linhas.por", kProgram);
    SPIKE_EXPECT(error.location == "linhas.por:9");
    SPIKE_EXPECT(error.source_text == "  a <- 2");
}

#else

void test_line_directives_do_not_change_the_program() {}

void test_compiler_error_points_to_por_line() {}

#endif

} // namespace

namespace spike_tests {

void run_codegen_line_tests() {
    test_source_lines_are_embedded_once();
    test_no_line_directives_by_default();
    test_line_directive_before_each_statement();
    test_line_directive_returns_to_generated_file();
    test_line_directives_do_not_change_the_program();
    test_compiler_error_points_to_por_line();
}

} // namespace spike_tests
