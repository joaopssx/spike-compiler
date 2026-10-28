#include "codegen/codegen_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <iostream>
#include <string>

namespace {

using spike_tests::contains;
using spike_tests::generate_from_source;

std::string generate(const std::string& procedures, const std::string& body) {
    return generate_from_source("algoritmo \"t\"\nvar\n  a : inteiro\n  v : vetor[1..3] de inteiro\n" +
                                procedures + "inicio\n" + body + "fimalgoritmo\n")
        .result.c_source;
}

void expect(const std::string& c, const std::string& piece) {
    const bool ok = contains(c, piece);
    if (!ok) std::cerr << "  expected C:\n" << piece << "\n";
    SPIKE_EXPECT(ok);
}

void test_prototypes_come_before_definitions() {
    const std::string c = generate("procedimento saudar(nome: caractere, vezes: inteiro)\ninicio\n"
                                   "fimprocedimento\nprocedimento Linha()\ninicio\n"
                                   "fimprocedimento\n",
                                   "  saudar(\"Ana\", 2)\n  linha()\n");
    expect(c, "spike_int v_v[3];\n\n"
              "static void p_saudar(int spike_line, spike_str v_nome, spike_int v_vezes);\n"
              "static void p_linha(int spike_line);\n\n"
              "static void p_saudar(int spike_line, spike_str v_nome, spike_int v_vezes) {\n"
              "    spike_enter(\"saudar\", spike_line);\n    spike_leave();\n}\n\n"
              "static void p_linha(int spike_line) {\n    spike_enter(\"Linha\", spike_line);\n    spike_leave();\n}\n\n"
              "int main(void) {\n");
    expect(c, "    p_saudar(12, \"Ana\", 2LL);\n    p_linha(13);\n");
}

void test_locals_start_with_default_values() {
    const std::string c = generate("procedimento p(x: real)\nvar\n  i : inteiro\n  t : caractere\n"
                                   "  ok : logico\n  w : vetor[0..4] de real\n"
                                   "  nomes : vetor[1..2] de caractere\ninicio\n  i <- 1\n"
                                   "fimprocedimento\n",
                                   "  p(a)\n");
    expect(c, "static void p_p(int spike_line, spike_real v_x) {\n"
              "    spike_enter(\"p\", spike_line);\n"
              "    spike_int v_i = 0LL;\n"
              "    spike_str v_t = \"\";\n"
              "    spike_bool v_ok = 0;\n"
              "    spike_real v_w[5] = {0};\n"
              "    spike_str v_nomes[2];\n"
              "    spike_fill_str(v_nomes, 2);\n"
              "    v_i = 1LL;\n"
              "    spike_leave();\n"
              "}\n");
    expect(c, "    p_p(16, ((spike_real)v_a));\n");
}

void test_local_shadows_global_array() {
    const std::string c = generate("procedimento p()\nvar\n  v : real\ninicio\n  v <- a\n"
                                   "fimprocedimento\nprocedimento q()\nvar\n"
                                   "  a : vetor[2..3] de inteiro\ninicio\n  a[3] <- v[1]\n"
                                   "fimprocedimento\n",
                                   "  v[2] <- 1\n  p()\n  q()\n");
    expect(c, "    spike_real v_v = 0.0;\n    v_v = ((spike_real)v_a);\n");
    expect(c, "    spike_int v_a[2] = {0};\n"
              "    v_a[spike_index(3LL, 2LL, 3LL, 15, \"a\")] = "
              "v_v[spike_index(1LL, 1LL, 3LL, 15, \"v\")];\n");
    expect(c, "    v_v[spike_index(2LL, 1LL, 3LL, 18, \"v\")] = 1LL;\n");
}

void test_recursive_call() {
    const std::string c = generate("procedimento conta(n: inteiro)\ninicio\n  se n > 0 entao\n"
                                   "    conta(n - 1)\n  fimse\nfimprocedimento\n",
                                   "  conta(3)\n");
    expect(c, "    if (v_n > 0LL) {\n        p_conta(8, spike_sub_int(v_n, 1LL));\n    }\n");
}

} // namespace

namespace spike_tests {

void run_codegen_procedure_tests() {
    test_prototypes_come_before_definitions();
    test_locals_start_with_default_values();
    test_local_shadows_global_array();
    test_recursive_call();
}

} // namespace spike_tests
