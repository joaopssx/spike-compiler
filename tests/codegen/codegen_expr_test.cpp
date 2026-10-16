#include "codegen/codegen_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {

using spike_tests::contains;
using spike_tests::generate_body;

void expect_line(const std::string& body, const std::string& expected) {
    const std::string c = generate_body(body);
    const bool ok = contains(c, "    " + expected + "\n");
    if (!ok) std::cerr << "  expected C line: " << expected << "\n  for: " << body;
    SPIKE_EXPECT(ok);
}

void test_codegen_integer_arithmetic() {
    expect_line("  a <- i + 2\n", "v_a = spike_add_int(v_i, 2LL);");
    expect_line("  a <- i - 2\n", "v_a = spike_sub_int(v_i, 2LL);");
    expect_line("  a <- i * (a + 1)\n", "v_a = spike_mul_int(v_i, spike_add_int(v_a, 1LL));");
    expect_line("  a <- -i\n", "v_a = spike_sub_int(0LL, v_i);");
    expect_line("  a <- -5\n", "v_a = (-5LL);");
}

void test_codegen_real_arithmetic_and_conversion() {
    expect_line("  r <- a\n", "v_r = ((spike_real)v_a);");
    expect_line("  r <- 7\n", "v_r = ((spike_real)7LL);");
    expect_line("  r <- a + 1.5\n", "v_r = (((spike_real)v_a) + 1.5);");
    expect_line("  r <- r * r - 0.1\n", "v_r = ((v_r * v_r) - 0.1);");
    expect_line("  r <- -r\n", "v_r = (-v_r);");
    expect_line("  r <- 3.0\n", "v_r = 3.0;");
}

void test_codegen_division_is_always_real() {
    expect_line("  r <- a / 2\n",
                "v_r = spike_div_real(((spike_real)v_a), ((spike_real)2LL), 8);");
    expect_line("  r <- r / 0.5\n", "v_r = spike_div_real(v_r, 0.5, 8);");
}

void test_codegen_div_mod_power() {
    expect_line("  a <- i div 2\n", "v_a = spike_div_int(v_i, 2LL, 8);");
    expect_line("  a <- i mod (a + 1)\n", "v_a = spike_mod_int(v_i, spike_add_int(v_a, 1LL), 8);");
    expect_line("  r <- 2 ^ 10\n", "v_r = spike_pow(((spike_real)2LL), ((spike_real)10LL), 8);");
    expect_line("  r <- r ^ 0.5\n", "v_r = spike_pow(v_r, 0.5, 8);");
    expect_line("  r <- -2 ^ 2\n", "v_r = (-spike_pow(((spike_real)2LL), ((spike_real)2LL), 8));");
}

void test_codegen_text() {
    expect_line("  t <- \"Olá, \\\"mundo\\\"\"\n", "v_t = \"Olá, \\\"mundo\\\"\";");
    expect_line("  t <- t + \" e \" + u\n",
                "v_t = spike_str_concat(spike_str_concat(v_t, \" e \"), v_u);");
}

void test_codegen_comparisons_and_logic() {
    for (const auto& [body, line] : std::vector<std::pair<std::string, std::string>>{
             {"  b <- t = \"a\"\n", "v_b = spike_str_eq(v_t, \"a\");"},
             {"  b <- t <> u\n", "v_b = (!spike_str_eq(v_t, v_u));"},
             {"  b <- t < u\n", "v_b = (spike_str_cmp(v_t, v_u) < 0);"},
             {"  b <- t >= u\n", "v_b = (spike_str_cmp(v_t, v_u) >= 0);"},
             {"  b <- a <> 1\n", "v_b = (v_a != 1LL);"},
             {"  b <- a < r\n", "v_b = (((spike_real)v_a) < v_r);"},
             {"  b <- b = c\n", "v_b = (v_b == v_c);"},
             {"  b <- b e nao c\n", "v_b = (v_b && (!v_c));"},
             {"  b <- (a > 1) ou falso\n", "v_b = ((v_a > 1LL) || 0);"},
             {"  b <- verdadeiro\n", "v_b = 1;"},
         }) {
        expect_line(body, line);
    }
}

} // namespace

namespace spike_tests {

void run_codegen_expr_tests() {
    test_codegen_integer_arithmetic();
    test_codegen_real_arithmetic_and_conversion();
    test_codegen_division_is_always_real();
    test_codegen_div_mod_power();
    test_codegen_text();
    test_codegen_comparisons_and_logic();
}

} // namespace spike_tests
