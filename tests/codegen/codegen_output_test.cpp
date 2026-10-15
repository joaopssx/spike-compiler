#include "codegen/codegen_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <iostream>
#include <string>

namespace {

using spike_tests::contains;
using spike_tests::generate_body;

void expect_lines(const std::string& body, const std::string& expected) {
    const std::string c = generate_body(body);
    const bool ok = contains(c, "    spike_init();\n" + expected + "    spike_flush();\n");
    if (!ok) std::cerr << "  expected C:\n" << expected << "  for: " << body;
    SPIKE_EXPECT(ok);
}

void test_escreva_each_type() {
    expect_lines("  escreva(a)\n", "    spike_write_int(v_a);\n");
    expect_lines("  escreva(r)\n", "    spike_write_real(v_r);\n");
    expect_lines("  escreva(t)\n", "    spike_write_str(v_t);\n");
    expect_lines("  escreva(b)\n", "    spike_write_bool(v_b);\n");
    expect_lines("  escreva(1.5)\n", "    spike_write_real(1.5);\n");
    expect_lines("  escreva(a > 1)\n", "    spike_write_bool((v_a > 1LL));\n");
}

void test_escreva_several_arguments_without_separator() {
    expect_lines("  escreva(\"a = \", a, \"; r = \", r)\n",
                 "    spike_write_str(\"a = \");\n"
                 "    spike_write_int(v_a);\n"
                 "    spike_write_str(\"; r = \");\n"
                 "    spike_write_real(v_r);\n");
}

void test_escreval_adds_newline() {
    expect_lines("  escreval(t, u)\n",
                 "    spike_write_str(v_t);\n"
                 "    spike_write_str(v_u);\n"
                 "    spike_write_newline();\n");
}

void test_empty_escreva_and_escreval() {
    expect_lines("  escreval()\n", "    spike_write_newline();\n");
    expect_lines("  escreva()\n", "");
}

void test_escreva_text_with_accent_and_quotes() {
    expect_lines("  escreval(\"Atenção: \\\"ok\\\"\")\n",
                 "    spike_write_str(\"Atenção: \\\"ok\\\"\");\n"
                 "    spike_write_newline();\n");
}

void test_escreva_compound_expression() {
    expect_lines("  escreval(a + i * 2)\n",
                 "    spike_write_int(spike_add_int(v_a, spike_mul_int(v_i, 2LL)));\n"
                 "    spike_write_newline();\n");
    expect_lines("  escreval(a / 2)\n",
                 "    spike_write_real(spike_div_real(((spike_real)v_a), ((spike_real)2LL), 8));\n"
                 "    spike_write_newline();\n");
}

void test_escreva_mixed_with_assignments() {
    expect_lines("  a <- 2\n  escreval(a)\n",
                 "    v_a = 2LL;\n"
                 "    spike_write_int(v_a);\n"
                 "    spike_write_newline();\n");
}

} // namespace

namespace spike_tests {

void run_codegen_output_tests() {
    test_escreva_each_type();
    test_escreva_several_arguments_without_separator();
    test_escreval_adds_newline();
    test_empty_escreva_and_escreval();
    test_escreva_text_with_accent_and_quotes();
    test_escreva_compound_expression();
    test_escreva_mixed_with_assignments();
}

} // namespace spike_tests
