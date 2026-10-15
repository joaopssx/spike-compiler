#include "parser/parser_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/ast.hpp"

#include <string>
#include <variant>

namespace {

using spike_tests::parse_source;

// ---- parser ---------------------------------------------------------------

void test_parser_empty_program() {
    auto r = parse_source("algoritmo \"vazio\"\ninicio\nfimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.name == "vazio");
    SPIKE_EXPECT(r.program.globals.empty());
    SPIKE_EXPECT(r.program.body.empty());
}

void test_parser_single_var_decl() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "var\n"
        "   x : inteiro\n"
        "inicio\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.globals.size() == 1);
    SPIKE_EXPECT(r.program.globals[0].name == "x");
    SPIKE_EXPECT(r.program.globals[0].type == spike::TypeKind::Integer);
    SPIKE_EXPECT(r.program.globals[0].initializer == nullptr);
}

void test_parser_multi_name_var_decl() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "var\n"
        "   a, b, c : real\n"
        "inicio\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.globals.size() == 3);
    SPIKE_EXPECT(r.program.globals[0].name == "a");
    SPIKE_EXPECT(r.program.globals[1].name == "b");
    SPIKE_EXPECT(r.program.globals[2].name == "c");
    for (const auto& g : r.program.globals) {
        SPIKE_EXPECT(g.type == spike::TypeKind::Real);
    }
}

void test_parser_multiple_var_lines() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "var\n"
        "   x : inteiro\n"
        "   nome : caractere\n"
        "   ok : logico\n"
        "inicio\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.globals.size() == 3);
    SPIKE_EXPECT(r.program.globals[0].type == spike::TypeKind::Integer);
    SPIKE_EXPECT(r.program.globals[1].type == spike::TypeKind::String);
    SPIKE_EXPECT(r.program.globals[2].type == spike::TypeKind::Bool);
}

void test_parser_E100_missing_algoritmo() {
    auto r = parse_source("inicio fimalgoritmo");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E100") != std::string::npos);
}

void test_parser_E101_missing_name() {
    auto r = parse_source("algoritmo inicio fimalgoritmo");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E101") != std::string::npos);
}

void test_parser_E102_missing_inicio() {
    auto r = parse_source("algoritmo \"t\"\nfimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E102") != std::string::npos);
}

void test_parser_E103_missing_fimalgoritmo() {
    auto r = parse_source("algoritmo \"t\"\ninicio\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E103") != std::string::npos);
}

void test_parser_E104_bad_type() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "var\n"
        "   x : numero\n"
        "inicio\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E104") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find("numero") != std::string::npos);
}

void test_parser_E105_missing_colon() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "var\n"
        "   x inteiro\n"
        "inicio\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E105") != std::string::npos);
}

void test_parser_recovers_to_next_line_after_var_error() {
    // Linha ruim, depois uma linha boa — a boa deve ser parseada.
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "var\n"
        "   x foobar lixo\n"            // erro: sem ':'
        "   y : inteiro\n"
        "inicio\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    // y deveria ter sido capturado mesmo depois do erro em x.
    bool found_y = false;
    for (const auto& g : r.program.globals) {
        if (g.name == "y" && g.type == spike::TypeKind::Integer) found_y = true;
    }
    SPIKE_EXPECT(found_y);
}

} // namespace

namespace spike_tests {

void run_parser_program_tests() {
    test_parser_empty_program();
    test_parser_single_var_decl();
    test_parser_multi_name_var_decl();
    test_parser_multiple_var_lines();
    test_parser_E100_missing_algoritmo();
    test_parser_E101_missing_name();
    test_parser_E102_missing_inicio();
    test_parser_E103_missing_fimalgoritmo();
    test_parser_E104_bad_type();
    test_parser_E105_missing_colon();
    test_parser_recovers_to_next_line_after_var_error();
}

} // namespace spike_tests
