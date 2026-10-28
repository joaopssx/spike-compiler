#include "codegen/codegen_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <iostream>
#include <string>

namespace {

using spike_tests::contains;
using spike_tests::generate_from_source;

const char* kSubprograms = "funcao proximo(): inteiro\n"
                           "inicio\n  x <- x + 1\n  retorne x\nfimfuncao\n"
                           "funcao conta(): inteiro\n"
                           "inicio\n  retorne proximo()\nfimfuncao\n"
                           "funcao divide(a, b: inteiro, var r: inteiro): inteiro\n"
                           "inicio\n  r <- a mod b\n  retorne a div b\nfimfuncao\n"
                           "funcao maximo(a, b: inteiro): inteiro\n"
                           "inicio\n  retorne a + b\nfimfuncao\n"
                           "funcao le(): inteiro\n"
                           "var\n  n : inteiro\ninicio\n  leia(n)\n  retorne n\nfimfuncao\n";

std::string generate(const std::string& body, const std::string& extra = "") {
    return generate_from_source("algoritmo \"t\"\nvar\n  x, y : inteiro\n"
                                "  v : vetor[1..3] de inteiro\n" +
                                std::string(kSubprograms) + extra + "inicio\n" + body +
                                "fimalgoritmo\n")
        .result.c_source;
}

void expect(const std::string& c, const std::string& piece) {
    const bool ok = contains(c, piece);
    if (!ok) std::cerr << "  expected C:\n" << piece << "\n";
    SPIKE_EXPECT(ok);
}

void expect_absent(const std::string& c, const std::string& piece) {
    const bool ok = !contains(c, piece);
    if (!ok) std::cerr << "  unexpected C:\n" << piece << "\n";
    SPIKE_EXPECT(ok);
}

void test_call_that_changes_a_variable_runs_before_the_read() {
    expect(generate("  escreval(divide(9, 3, y) + y)\n"),
           "    {\n"
           "        spike_int spike_tmp_1 = f_divide(31, 9LL, 3LL, &v_y);\n"
           "        spike_write_int(spike_add_int(spike_tmp_1, v_y));\n"
           "    }\n"
           "    spike_write_newline();\n");
}

void test_read_on_the_left_keeps_the_old_value() {
    expect(generate("  y <- y + divide(9, 3, y)\n"),
           "    {\n"
           "        spike_int spike_tmp_1 = v_y;\n"
           "        v_y = spike_add_int(spike_tmp_1, f_divide(31, 9LL, 3LL, &v_y));\n"
           "    }\n");
}

void test_two_calls_with_effects_run_left_to_right() {
    expect(generate("  y <- proximo() - proximo()\n"),
           "        spike_int spike_tmp_1 = f_proximo(31);\n"
           "        v_y = spike_sub_int(spike_tmp_1, f_proximo(31));\n");
    expect(generate("  y <- le() - le()\n"),
           "        spike_int spike_tmp_1 = f_le(31);\n"
           "        v_y = spike_sub_int(spike_tmp_1, f_le(31));\n");
}

void test_effect_of_a_called_function_counts() {
    expect(generate("  y <- x + conta()\n"),
           "        spike_int spike_tmp_1 = v_x;\n"
           "        v_y = spike_add_int(spike_tmp_1, f_conta(31));\n");
}

void test_functions_without_effects_stay_on_one_line() {
    const std::string c =
        generate("  y <- maximo(maximo(x, y), x) + fib(x)\n",
                 "funcao fib(n: inteiro): inteiro\n"
                 "inicio\n  retorne fib(n - 1) + fib(n - 2)\nfimfuncao\n");
    expect(c, "    v_y = spike_add_int(f_maximo(35, f_maximo(35, v_x, v_y), v_x), "
              "f_fib(35, v_x));\n");
    expect(c, "spike_int spike_result = spike_add_int(f_fib(32, spike_sub_int(v_n, 1LL)), "
              "f_fib(32, spike_sub_int(v_n, 2LL)));\n");
    expect_absent(c, "spike_tmp_");
}

void test_local_value_parameter_is_never_changed_by_a_call() {
    expect(generate("  y <- soma(1)\n", "funcao soma(n: inteiro): inteiro\n"
                                        "inicio\n  retorne n + proximo()\nfimfuncao\n"),
           "spike_int spike_result = spike_add_int(v_n, f_proximo(32));\n");
}

void test_local_passed_by_reference_can_change() {
    expect(generate("  y <- dobro(1)\n", "funcao dobro(n: inteiro): inteiro\n"
                                         "inicio\n  retorne n + divide(n, 2, n)\nfimfuncao\n"),
           "        spike_int spike_tmp_1 = v_n;\n"
           "        spike_int spike_result = spike_add_int(spike_tmp_1, "
           "f_divide(32, v_n, 2LL, &v_n));\n");
}

void test_index_of_the_target_is_read_first() {
    expect(generate("  v[x] <- divide(9, 2, x)\n"),
           "        spike_int spike_tmp_1 = v_x;\n"
           "        v_v[spike_index(spike_tmp_1, 1LL, 3LL, 31, \"v\")] = "
           "f_divide(31, 9LL, 2LL, &v_x);\n");
}

void test_right_side_of_e_and_ou_still_stops_early() {
    expect(generate("  se x > 1 ou proximo() + x > 0 entao\n    y <- 1\n  fimse\n"),
           "    {\n"
           "        spike_bool spike_tmp_1 = (v_x > 1LL);\n"
           "        if (!spike_tmp_1) {\n"
           "            spike_int spike_tmp_2 = f_proximo(31);\n"
           "            spike_tmp_1 = (spike_add_int(spike_tmp_2, v_x) > 0LL);\n"
           "        }\n"
           "        if (spike_tmp_1) {\n"
           "            v_y = 1LL;\n"
           "        }\n"
           "    }\n");
    expect(generate("  se x > 1 e proximo() > 0 entao\n    y <- 1\n  fimse\n"),
           "    if ((v_x > 1LL) && (f_proximo(31) > 0LL)) {\n");
}

void test_loop_conditions_are_evaluated_on_every_turn() {
    expect(generate("  enquanto x < proximo() faca\n    y <- 2\n  fimenquanto\n"),
           "    while (1) {\n"
           "        spike_int spike_tmp_1 = v_x;\n"
           "        if (!(spike_tmp_1 < f_proximo(31))) break;\n"
           "        v_y = 2LL;\n"
           "    }\n");
    expect(generate("  repita\n    y <- 3\n  ate x = proximo()\n"),
           "    while (1) {\n"
           "        v_y = 3LL;\n"
           "        spike_int spike_tmp_1 = v_x;\n"
           "        if (spike_tmp_1 == f_proximo(33)) break;\n"
           "    }\n");
}

void test_element_passed_by_reference_is_located_first() {
    expect(generate("  y <- guarda(v[x], proximo())\n",
                    "funcao guarda(var r: inteiro, valor: inteiro): inteiro\n"
                    "inicio\n  r <- valor\n  retorne valor\nfimfuncao\n"),
           "        spike_int* spike_tmp_1 = &v_v[spike_index(v_x, 1LL, 3LL, 36, \"v\")];\n"
           "        v_y = f_guarda(36, spike_tmp_1, f_proximo(36));\n");
}

} // namespace

namespace spike_tests {

void run_codegen_order_tests() {
    test_call_that_changes_a_variable_runs_before_the_read();
    test_read_on_the_left_keeps_the_old_value();
    test_two_calls_with_effects_run_left_to_right();
    test_effect_of_a_called_function_counts();
    test_functions_without_effects_stay_on_one_line();
    test_local_value_parameter_is_never_changed_by_a_call();
    test_local_passed_by_reference_can_change();
    test_index_of_the_target_is_read_first();
    test_right_side_of_e_and_ou_still_stops_early();
    test_loop_conditions_are_evaluated_on_every_turn();
    test_element_passed_by_reference_is_located_first();
}

} // namespace spike_tests
