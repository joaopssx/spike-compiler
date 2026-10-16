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
    if (!ok) {
        const auto start = c.find("int main(void)");
        std::cerr << "  expected:\n" << expected << "  got:\n"
                  << (start == std::string::npos ? c : c.substr(start));
    }
    SPIKE_EXPECT(ok);
}

void test_escolha_on_variable_uses_if_chain() {
    expect_main("  escolha a\n    caso 1\n      i <- 1\n    caso 2, 3\n    caso 4 ate 6\n"
                "      i <- 4\n    outrocaso\n      i <- 0\n  fimescolha\n",
                "    if (v_a == 1LL) {\n"
                "        v_i = 1LL;\n"
                "    } else if (v_a == 2LL || v_a == 3LL) {\n"
                "    } else if (v_a >= 4LL && v_a <= 6LL) {\n"
                "        v_i = 4LL;\n"
                "    } else {\n"
                "        v_i = 0LL;\n"
                "    }\n");
}

void test_escolha_on_expression_evaluates_once() {
    expect_main("  escolha a mod 3\n    caso 0, 1 ate 2\n      i <- 1\n  fimescolha\n",
                "    {\n"
                "        spike_int spike_sel_1 = spike_mod_int(v_a, 3LL, 8);\n"
                "        if (spike_sel_1 == 0LL || (spike_sel_1 >= 1LL && spike_sel_1 <= 2LL)) {\n"
                "            v_i = 1LL;\n"
                "        }\n"
                "    }\n");
}

void test_escolha_on_text() {
    expect_main("  escolha t\n    caso \"a\", \"b\"\n      i <- 1\n  fimescolha\n",
                "    if (spike_str_eq(v_t, \"a\") || spike_str_eq(v_t, \"b\")) {\n"
                "        v_i = 1LL;\n"
                "    }\n");
}

void test_escolha_with_only_default() {
    expect_main("  escolha a\n    outrocaso\n      i <- 2\n  fimescolha\n", "    v_i = 2LL;\n");
}

void test_break_inside_escolha_leaves_the_loop() {
    expect_main("  enquanto b faca\n    escolha a\n      caso 1\n        interrompa\n"
                "    fimescolha\n  fimenquanto\n",
                "    while (v_b) {\n"
                "        if (v_a == 1LL) {\n"
                "            break;\n"
                "        }\n"
                "    }\n");
}

} // namespace

namespace spike_tests {

void run_codegen_escolha_tests() {
    test_escolha_on_variable_uses_if_chain();
    test_escolha_on_expression_evaluates_once();
    test_escolha_on_text();
    test_escolha_with_only_default();
    test_break_inside_escolha_leaves_the_loop();
}

} // namespace spike_tests
