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

void test_break_in_while() {
    expect_main("  enquanto b faca\n    interrompa\n  fimenquanto\n",
                "    while (v_b) {\n"
                "        break;\n"
                "    }\n");
}

void test_break_in_repeat() {
    expect_main("  repita\n    se a > 3 entao\n      interrompa\n    fimse\n  ate b\n",
                "    do {\n"
                "        if (v_a > 3LL) {\n"
                "            break;\n"
                "        }\n"
                "    } while (!(v_b));\n");
}

void test_break_in_para_leaves_the_loop_not_the_block() {
    expect_main("  para i de 1 ate a faca\n    interrompa\n  fimpara\n",
                "    {\n"
                "        spike_int spike_to_1 = v_a;\n"
                "        v_i = 1LL;\n"
                "        while (v_i <= spike_to_1) {\n"
                "            break;\n"
                "            if (!spike_for_advance(&v_i, 1LL)) break;\n"
                "        }\n"
                "    }\n");
}

} // namespace

namespace spike_tests {

void run_codegen_break_tests() {
    test_break_in_while();
    test_break_in_repeat();
    test_break_in_para_leaves_the_loop_not_the_block();
}

} // namespace spike_tests
