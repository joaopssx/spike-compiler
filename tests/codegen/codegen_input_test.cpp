#include "codegen/codegen_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <iostream>
#include <string>

namespace {

using spike_tests::contains;
using spike_tests::generate_body;

void expect_main(const std::string& body, const std::string& expected) {
    const std::string c = generate_body(body);
    const bool ok = contains(c, "    spike_init();\n" + expected + "    spike_flush();\n");
    if (!ok) std::cerr << "  expected:\n" << expected << "  for: " << body;
    SPIKE_EXPECT(ok);
}

void test_leia_each_type() {
    expect_main("  leia(a)\n", "    v_a = spike_read_int(8, \"a\");\n");
    expect_main("  leia(r)\n", "    v_r = spike_read_real(8, \"r\");\n");
    expect_main("  leia(t)\n", "    v_t = spike_read_str(8, \"t\");\n");
    expect_main("  leia(b)\n", "    v_b = spike_read_bool(8, \"b\");\n");
}

void test_leia_several_variables_in_order() {
    expect_main("  leia(a, R, t)\n",
                "    v_a = spike_read_int(8, \"a\");\n"
                "    v_r = spike_read_real(8, \"R\");\n"
                "    v_t = spike_read_str(8, \"t\");\n");
}

void test_leia_uses_line_of_each_name() {
    expect_main("  leia(a,\n       i)\n",
                "    v_a = spike_read_int(8, \"a\");\n"
                "    v_i = spike_read_int(9, \"i\");\n");
}

void test_leia_between_writes() {
    expect_main("  escreva(\"Digite: \")\n  leia(a)\n  escreval(a * 2)\n",
                "    spike_write_str(\"Digite: \");\n"
                "    v_a = spike_read_int(9, \"a\");\n"
                "    spike_write_int(spike_mul_int(v_a, 2LL));\n"
                "    spike_write_newline();\n");
}

} // namespace

namespace spike_tests {

void run_codegen_input_tests() {
    test_leia_each_type();
    test_leia_several_variables_in_order();
    test_leia_uses_line_of_each_name();
    test_leia_between_writes();
}

} // namespace spike_tests
