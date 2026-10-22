#include "codegen/codegen_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <iostream>
#include <string>

namespace {

using spike_tests::contains;
using spike_tests::generate_body;

void expect(const std::string& c, const std::string& piece) {
    const bool ok = contains(c, piece);
    if (!ok) std::cerr << "  expected C:\n" << piece << "\n";
    SPIKE_EXPECT(ok);
}

void test_calls_use_the_table_c_name() {
    const std::string c = generate_body("  a <- arredonda(r)\n  r <- raiz(a) + abs(r)\n"
                                        "  a <- abs(a - 1)\n  r <- seno(r)\n");
    expect(c, "    v_a = spike_round(v_r, 8);\n");
    expect(c, "    v_r = (spike_sqrt(((spike_real)v_a), 9) + fabs(v_r));\n");
    expect(c, "    v_a = spike_abs_int(spike_sub_int(v_a, 1LL));\n");
    expect(c, "    v_r = sin(v_r);\n");
}

void test_integer_arguments_become_real() {
    expect(generate_body("  r <- pot(2, a)\n"),
           "    v_r = spike_pow(((spike_real)2LL), ((spike_real)v_a), 8);\n");
}

void test_text_builtins() {
    const std::string c = generate_body("  a <- tamanho(t) + pos(\"x\", u)\n  t <- copia(t, a, 2)\n"
                                        "  t <- paraTexto(r) + paraTexto(a) + paraTexto(b)\n"
                                        "  a <- paraInteiro(t)\n  b <- ehNumero(u)\n");
    expect(c, "    v_a = spike_add_int(spike_str_length(v_t), spike_str_find(\"x\", v_u));\n");
    expect(c, "    v_t = spike_str_slice(v_t, v_a, 2LL, 9);\n");
    expect(c, "spike_real_to_str(v_r), spike_int_to_str(v_a)), spike_bool_to_str(v_b));\n");
    expect(c, "    v_a = spike_str_to_int(v_t, 11);\n");
    expect(c, "    v_b = spike_str_is_number(v_u);\n");
}

void test_builtin_called_as_command() {
    expect(generate_body("  aleatorio(1, 6)\n"), "    spike_random(1LL, 6LL, 8);\n");
}

} // namespace

namespace spike_tests {

void run_codegen_builtin_tests() {
    test_calls_use_the_table_c_name();
    test_integer_arguments_become_real();
    test_builtin_called_as_command();
    test_text_builtins();
}

} // namespace spike_tests
