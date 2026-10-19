#include "codegen/codegen_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <iostream>
#include <string>

namespace {

using spike_tests::contains;
using spike_tests::generate_from_source;

std::string generate(const std::string& functions, const std::string& body) {
    return generate_from_source("algoritmo \"t\"\nvar\n  a : inteiro\n  r : real\n" + functions +
                                "inicio\n" + body + "fimalgoritmo\n")
        .result.c_source;
}

void expect(const std::string& c, const std::string& piece) {
    const bool ok = contains(c, piece);
    if (!ok) std::cerr << "  expected C:\n" << piece << "\n";
    SPIKE_EXPECT(ok);
}

const char* kFatorial = "funcao fatorial(n: inteiro): inteiro\n"
                        "inicio\n"
                        "  se n <= 1 entao\n"
                        "    retorne 1\n"
                        "  fimse\n"
                        "  retorne n * fatorial(n - 1)\n"
                        "fimfuncao\n";

void test_function_definition() {
    const std::string c = generate(kFatorial, "  a <- fatorial(5)\n");
    expect(c, "static spike_int f_fatorial(spike_int v_n);\n\n"
              "static spike_int f_fatorial(spike_int v_n) {\n"
              "    spike_enter(5);\n"
              "    if (v_n <= 1LL) {\n"
              "        {\n"
              "            spike_int spike_result = 1LL;\n"
              "            spike_leave();\n"
              "            return spike_result;\n"
              "        }\n"
              "    }\n"
              "    {\n"
              "        spike_int spike_result = spike_mul_int(v_n, f_fatorial(spike_sub_int(v_n, "
              "1LL)));\n"
              "        spike_leave();\n"
              "        return spike_result;\n"
              "    }\n"
              "    spike_missing_return(11, \"fatorial\");\n"
              "    return 0LL;\n"
              "}\n");
    expect(c, "    v_a = f_fatorial(5LL);\n");
}

void test_return_converts_integer_to_real() {
    const std::string c = generate("funcao Metade(x: real): real\ninicio\n  retorne a\nfimfuncao\n",
                                   "  r <- metade(a) + metade(2.5)\n");
    expect(c, "static spike_real f_metade(spike_real v_x) {\n");
    expect(c, "        spike_real spike_result = ((spike_real)v_a);\n");
    expect(c, "    spike_missing_return(8, \"Metade\");\n    return 0.0;\n");
    expect(c, "(f_metade(((spike_real)v_a)) + f_metade(2.5))");
}

void test_function_called_as_statement() {
    expect(generate(kFatorial, "  fatorial(3)\n"), "    f_fatorial(3LL);\n");
}

void test_text_and_logical_functions() {
    const std::string c =
        generate("funcao nome(): caractere\ninicio\n  retorne \"Ana\"\nfimfuncao\n"
                 "funcao ok(): logico\ninicio\n  retorne a > 0\nfimfuncao\n",
                 "  se ok() entao\n    escreval(nome())\n  fimse\n");
    expect(c, "static spike_str f_nome(void);\nstatic spike_bool f_ok(void);\n");
    expect(c, "    spike_missing_return(8, \"nome\");\n    return \"\";\n");
    expect(c, "    if (f_ok()) {\n        spike_write_str(f_nome());\n");
}

void test_procedure_retorne_leaves_early() {
    const std::string c = generate("procedimento p()\ninicio\n  se a > 0 entao\n    retorne\n"
                                   "  fimse\n  escreval(a)\nfimprocedimento\n",
                                   "  p()\n");
    expect(c, "static void p_p(void) {\n"
              "    spike_enter(5);\n"
              "    if (v_a > 0LL) {\n"
              "        spike_leave();\n"
              "        return;\n"
              "    }\n"
              "    spike_write_int(v_a);\n"
              "    spike_write_newline();\n"
              "    spike_leave();\n"
              "}\n");
}

} // namespace

namespace spike_tests {

void run_codegen_function_tests() {
    test_function_definition();
    test_return_converts_integer_to_real();
    test_function_called_as_statement();
    test_text_and_logical_functions();
    test_procedure_retorne_leaves_early();
}

} // namespace spike_tests
