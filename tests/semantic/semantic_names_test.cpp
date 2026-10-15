#include "semantic/semantic_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <string>

namespace {

using spike_tests::analyze_source;
using spike_tests::count_occurrences;
using spike_tests::program_with;

void test_semantic_declared_and_used_is_clean() {
    auto r = analyze_source(program_with("  x, y : inteiro\n",
                                         "  x <- 1\n  y <- x + 2\n  escreval(y)\n"));
    SPIKE_EXPECT(!r.parse_error);
    SPIKE_EXPECT(!r.had_error);
    SPIKE_EXPECT(r.diag_output.empty());
}

void test_semantic_undeclared_assignment_target() {
    auto r = analyze_source(program_with("  x : inteiro\n", "  y <- 1\n"));
    SPIKE_EXPECT(r.had_error);
    SPIKE_EXPECT(r.diag_output.find("E700") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find("variável 'y' não foi declarada") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find(":5:3") != std::string::npos);
}

void test_semantic_undeclared_in_expression() {
    auto r = analyze_source(program_with("  x : inteiro\n", "  x <- z * 2\n"));
    SPIKE_EXPECT(r.diag_output.find("'z' não foi declarada") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find(":5:8") != std::string::npos);
}

void test_semantic_undeclared_in_escreva() {
    auto r = analyze_source(program_with("", "  escreval(\"oi \", nome)\n"));
    SPIKE_EXPECT(r.diag_output.find("'nome' não foi declarada") != std::string::npos);
}

void test_semantic_undeclared_in_leia_points_at_name() {
    auto r = analyze_source(program_with("  a : inteiro\n", "  leia(a, b)\n"));
    SPIKE_EXPECT(r.diag_output.find("'b' não foi declarada") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find(":5:11") != std::string::npos);
    SPIKE_EXPECT(count_occurrences(r.diag_output, "E700") == 1);
}

void test_semantic_undeclared_para_variable_and_limits() {
    auto r = analyze_source(program_with("  n : inteiro\n",
                                         "  para i de 1 ate fim faca\n  fimpara\n"));
    SPIKE_EXPECT(r.diag_output.find("'i' não foi declarada") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find(":5:8") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find("'fim' não foi declarada") != std::string::npos);
}

void test_semantic_undeclared_in_conditions() {
    auto r = analyze_source(program_with(
        "",
        "  se a > 1 entao\n  fimse\n"
        "  enquanto b faca\n  fimenquanto\n"
        "  repita\n  ate c\n"));
    SPIKE_EXPECT(count_occurrences(r.diag_output, "E700") == 3);
    SPIKE_EXPECT(r.diag_output.find("'a'") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find("'b'") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find("'c'") != std::string::npos);
}

void test_semantic_undeclared_inside_nested_blocks() {
    auto r = analyze_source(program_with(
        "  x : inteiro\n",
        "  se x > 0 entao\n    enquanto x > 0 faca\n      w <- 1\n    fimenquanto\n"
        "  senao\n    escreval(v)\n  fimse\n"));
    SPIKE_EXPECT(count_occurrences(r.diag_output, "E700") == 2);
}

void test_semantic_undeclared_reported_once_per_line() {
    auto r = analyze_source(program_with("", "  escreval(q, q + q)\n  escreval(q)\n"));
    SPIKE_EXPECT(count_occurrences(r.diag_output, "E700") == 2);
}

void test_semantic_reports_every_name_error() {
    auto r = analyze_source(program_with("  x : inteiro\n  x : real\n",
                                         "  a <- 1\n  b <- 2\n"));
    SPIKE_EXPECT(count_occurrences(r.diag_output, "E701") == 1);
    SPIKE_EXPECT(count_occurrences(r.diag_output, "E700") == 2);
}

void test_semantic_duplicate_declaration() {
    auto r = analyze_source(program_with("  x : inteiro\n  x : real\n", ""));
    SPIKE_EXPECT(r.had_error);
    SPIKE_EXPECT(r.diag_output.find("E701") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find("variável 'x' já foi declarada") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find("está na linha 3") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find(":4:3") != std::string::npos);
}

void test_semantic_duplicate_in_same_line() {
    auto r = analyze_source(program_with("  a, b, a : inteiro\n", ""));
    SPIKE_EXPECT(count_occurrences(r.diag_output, "E701") == 1);
}

void test_semantic_names_ignore_case() {
    auto r = analyze_source(program_with("  Idade : inteiro\n",
                                         "  IDADE <- 18\n  escreval(idade)\n"));
    SPIKE_EXPECT(!r.had_error);
}

void test_semantic_duplicate_with_different_case() {
    auto r = analyze_source(program_with("  total : inteiro\n  TOTAL : real\n", ""));
    SPIKE_EXPECT(r.diag_output.find("E701") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find("'TOTAL' já foi declarada") != std::string::npos);
}

void test_semantic_program_without_var() {
    auto ok = analyze_source("algoritmo \"t\"\ninicio\n  escreval(\"oi\")\nfimalgoritmo\n");
    SPIKE_EXPECT(!ok.had_error);
    SPIKE_EXPECT(ok.symbols.size() == 0);

    auto bad = analyze_source("algoritmo \"t\"\ninicio\n  x <- 1\nfimalgoritmo\n");
    SPIKE_EXPECT(bad.diag_output.find("E700") != std::string::npos);
}

} // namespace

namespace spike_tests {

void run_semantic_name_tests() {
    test_semantic_declared_and_used_is_clean();
    test_semantic_undeclared_assignment_target();
    test_semantic_undeclared_in_expression();
    test_semantic_undeclared_in_escreva();
    test_semantic_undeclared_in_leia_points_at_name();
    test_semantic_undeclared_para_variable_and_limits();
    test_semantic_undeclared_in_conditions();
    test_semantic_undeclared_inside_nested_blocks();
    test_semantic_undeclared_reported_once_per_line();
    test_semantic_reports_every_name_error();
    test_semantic_duplicate_declaration();
    test_semantic_duplicate_in_same_line();
    test_semantic_names_ignore_case();
    test_semantic_duplicate_with_different_case();
    test_semantic_program_without_var();
}

} // namespace spike_tests
