#include "codegen/codegen_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <iostream>
#include <string>

namespace {

using spike_tests::contains;
using spike_tests::generate_from_source;

std::string generate(const std::string& subprograms, const std::string& body) {
    return generate_from_source("algoritmo \"t\"\nvar\n  x, y : inteiro\n"
                                "  v : vetor[1..3] de inteiro\n" +
                                subprograms + "inicio\n" + body + "fimalgoritmo\n")
        .result.c_source;
}

void expect(const std::string& c, const std::string& piece) {
    const bool ok = contains(c, piece);
    if (!ok) std::cerr << "  expected C:\n" << piece << "\n";
    SPIKE_EXPECT(ok);
}

const char* kTroca = "procedimento troca(var a: inteiro, var b: inteiro)\n"
                     "var\n"
                     "  t : inteiro\n"
                     "inicio\n"
                     "  t <- a\n"
                     "  a <- b\n"
                     "  b <- t\n"
                     "fimprocedimento\n";

void test_reference_parameters_are_pointers() {
    const std::string c = generate(kTroca, "  troca(x, y)\n");
    expect(c, "static void p_troca(int spike_line, spike_int* v_a, spike_int* v_b);\n");
    expect(c, "    spike_int v_t = 0LL;\n"
              "    v_t = (*v_a);\n"
              "    (*v_a) = (*v_b);\n"
              "    (*v_b) = v_t;\n");
    expect(c, "    p_troca(14, &v_x, &v_y);\n");
}

void test_element_address_is_checked_first() {
    expect(generate(kTroca, "  troca(v[1], v[x])\n"),
           "    p_troca(14, &v_v[spike_index(1LL, 1LL, 3LL, 14, \"v\")], "
           "&v_v[spike_index(v_x, 1LL, 3LL, 14, \"v\")]);\n");
}

void test_reference_passed_on_and_used_everywhere() {
    const std::string c = generate(std::string(kTroca) +
                                       "funcao conta(var n: inteiro, salto: inteiro): inteiro\n"
                                       "inicio\n  troca(n, x)\n  leia(n)\n"
                                       "  para n de 1 ate salto faca\n  fimpara\n"
                                       "  retorne n * 2\nfimfuncao\n",
                                   "  y <- conta(x, 3)\n");
    expect(c, "static spike_int f_conta(int spike_line, spike_int* v_n, spike_int v_salto) {\n");
    expect(c, "    p_troca(15, v_n, &v_x);\n");
    expect(c, "    (*v_n) = spike_read_int(16, \"n\");\n");
    expect(c, "        spike_int spike_to_1 = v_salto;\n        (*v_n) = 1LL;\n"
              "        while ((*v_n) <= spike_to_1) {\n");
    expect(c, "if (!spike_for_advance(&(*v_n), 1LL)) break;\n");
    expect(c, "spike_int spike_result = spike_mul_int((*v_n), 2LL);\n");
    expect(c, "    v_y = f_conta(22, &v_x, 3LL);\n");
}

void test_value_parameter_with_same_name_is_not_a_pointer() {
    const std::string c = generate(std::string(kTroca) +
                                       "procedimento mostra(a: inteiro)\ninicio\n  escreval(a)\n"
                                       "fimprocedimento\n",
                                   "  mostra(x)\n");
    expect(c, "static void p_mostra(int spike_line, spike_int v_a) {\n"
              "    spike_enter(\"mostra\", spike_line);\n"
              "    spike_write_int(v_a);\n");
}

} // namespace

namespace spike_tests {

void run_codegen_reference_tests() {
    test_reference_parameters_are_pointers();
    test_element_address_is_checked_first();
    test_reference_passed_on_and_used_everywhere();
    test_value_parameter_with_same_name_is_not_a_pointer();
}

} // namespace spike_tests
