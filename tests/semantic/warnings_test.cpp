#include "semantic/semantic_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <string>

namespace {

using spike_tests::count_occurrences;
using spike_tests::program_with;
using spike_tests::warnings_of;

bool has(const std::string& out, const std::string& code) {
    return out.find("aviso[" + code + "]") != std::string::npos;
}

bool none(const std::string& out) {
    return out.find("aviso[") == std::string::npos;
}

void test_unused_variable_is_w001() {
    const std::string out = warnings_of(program_with("  x, y : inteiro\n", "  y <- 1\n  escreval(y)\n"));
    SPIKE_EXPECT(has(out, "W001"));
    SPIKE_EXPECT(count_occurrences(out, "aviso[") == 1);
    SPIKE_EXPECT(out.find("'x' foi declarada, mas nunca é usada") != std::string::npos);
    SPIKE_EXPECT(out.find("<test>:3:3") != std::string::npos);
    SPIKE_EXPECT(out.find("dica:") != std::string::npos);
}

void test_read_but_never_assigned_is_w002() {
    const std::string out = warnings_of(program_with("  x : inteiro\n", "  escreval(x)\n"));
    SPIKE_EXPECT(has(out, "W002"));
    SPIKE_EXPECT(count_occurrences(out, "aviso[") == 1);
}

void test_leia_counts_as_assignment() {
    SPIKE_EXPECT(none(warnings_of(program_with("  x : inteiro\n", "  leia(x)\n  escreval(x)\n"))));
    SPIKE_EXPECT(none(warnings_of(
        program_with("  v : vetor[1..2] de inteiro\n", "  leia(v[1])\n  escreval(v[1])\n"))));
}

void test_assigned_but_never_read_is_w003() {
    const std::string out = warnings_of(program_with("  x : inteiro\n", "  x <- 5\n"));
    SPIKE_EXPECT(has(out, "W003"));
    SPIKE_EXPECT(count_occurrences(out, "aviso[") == 1);
    SPIKE_EXPECT(has(warnings_of(program_with("  x : inteiro\n", "  leia(x)\n")), "W003"));
}

void test_para_counter_counts_as_read() {
    SPIKE_EXPECT(none(warnings_of(
        program_with("  i : inteiro\n", "  para i de 1 ate 3 faca\n    escreval(\"oi\")\n  fimpara\n"))));
}

void test_reference_argument_counts_as_read_and_written() {
    const std::string src =
        "algoritmo \"t\"\nvar\n  a : inteiro\n"
        "procedimento zera(var n: inteiro)\ninicio\n  n <- 0\nfimprocedimento\n"
        "inicio\n  zera(a)\nfimalgoritmo\n";
    SPIKE_EXPECT(none(warnings_of(src)));
}

void test_parameters_are_not_variable_warnings() {
    const std::string src =
        "algoritmo \"t\"\nprocedimento p(n: inteiro)\ninicio\n  escreval(\"oi\")\nfimprocedimento\n"
        "inicio\n  p(1)\nfimalgoritmo\n";
    SPIKE_EXPECT(none(warnings_of(src)));
}

void test_unused_local_is_w001() {
    const std::string src =
        "algoritmo \"t\"\nprocedimento p()\nvar\n  sobra : inteiro\ninicio\n  escreval(\"oi\")\n"
        "fimprocedimento\ninicio\n  p()\nfimalgoritmo\n";
    const std::string out = warnings_of(src);
    SPIKE_EXPECT(has(out, "W001"));
    SPIKE_EXPECT(out.find("'sobra'") != std::string::npos);
}

void test_changing_para_counter_is_w004() {
    const std::string out = warnings_of(program_with(
        "  i : inteiro\n", "  para i de 1 ate 10 faca\n    i <- i + 1\n  fimpara\n"));
    SPIKE_EXPECT(has(out, "W004"));
    SPIKE_EXPECT(out.find("<test>:6:5") != std::string::npos);
    SPIKE_EXPECT(none(warnings_of(program_with(
        "  i, s : inteiro\n",
        "  s <- 0\n  para i de 1 ate 10 faca\n    s <- s + i\n  fimpara\n  escreval(s)\n"))));
}

void test_local_shadowing_global_is_w005() {
    const std::string src =
        "algoritmo \"t\"\nvar\n  x : inteiro\nprocedimento p()\nvar\n  x : inteiro\ninicio\n"
        "  x <- 1\n  escreval(x)\nfimprocedimento\ninicio\n  x <- 2\n  p()\n  escreval(x)\n"
        "fimalgoritmo\n";
    const std::string out = warnings_of(src);
    SPIKE_EXPECT(has(out, "W005"));
    SPIKE_EXPECT(count_occurrences(out, "aviso[") == 1);
    SPIKE_EXPECT(out.find("<test>:6:3") != std::string::npos);
}

void test_parameter_with_global_name_is_not_w005() {
    const std::string src =
        "algoritmo \"t\"\nvar\n  n : inteiro\nfuncao dobro(n: inteiro): inteiro\ninicio\n"
        "  retorne n * 2\nfimfuncao\ninicio\n  n <- 3\n  escreval(dobro(n))\nfimalgoritmo\n";
    SPIKE_EXPECT(none(warnings_of(src)));
}

void test_function_without_return_is_w006() {
    const std::string src =
        "algoritmo \"t\"\nfuncao sinal(n: inteiro): inteiro\ninicio\n  se n > 0 entao\n"
        "    retorne 1\n  fimse\nfimfuncao\ninicio\n  escreval(sinal(2))\nfimalgoritmo\n";
    const std::string out = warnings_of(src);
    SPIKE_EXPECT(has(out, "W006"));
    SPIKE_EXPECT(out.find("<test>:2:") != std::string::npos);
    const std::string complete =
        "algoritmo \"t\"\nfuncao sinal(n: inteiro): inteiro\ninicio\n  se n > 0 entao\n"
        "    retorne 1\n  senao\n    retorne 0\n  fimse\nfimfuncao\ninicio\n"
        "  escreval(sinal(2))\nfimalgoritmo\n";
    SPIKE_EXPECT(none(warnings_of(complete)));
}

void test_no_warnings_when_there_are_errors() {
    const std::string out = warnings_of(program_with("  x : inteiro\n", "  y <- 1\n"));
    SPIKE_EXPECT(none(out));
}

} // namespace

namespace spike_tests {

void run_warning_tests() {
    test_unused_variable_is_w001();
    test_read_but_never_assigned_is_w002();
    test_leia_counts_as_assignment();
    test_assigned_but_never_read_is_w003();
    test_para_counter_counts_as_read();
    test_reference_argument_counts_as_read_and_written();
    test_parameters_are_not_variable_warnings();
    test_unused_local_is_w001();
    test_changing_para_counter_is_w004();
    test_local_shadowing_global_is_w005();
    test_parameter_with_global_name_is_not_w005();
    test_function_without_return_is_w006();
    test_no_warnings_when_there_are_errors();
}

} // namespace spike_tests
