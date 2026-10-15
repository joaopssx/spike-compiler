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

void test_se_with_and_without_senao() {
    expect_main("  se a > 1 entao\n    a <- 0\n  fimse\n",
                "    if (v_a > 1LL) {\n"
                "        v_a = 0LL;\n"
                "    }\n");
    expect_main("  se b entao\n    escreval(\"sim\")\n  senao\n    escreval(\"nao\")\n  fimse\n",
                "    if (v_b) {\n"
                "        spike_write_str(\"sim\");\n"
                "        spike_write_newline();\n"
                "    } else {\n"
                "        spike_write_str(\"nao\");\n"
                "        spike_write_newline();\n"
                "    }\n");
    expect_main("  se (a > 1) e b entao\n  fimse\n",
                "    if ((v_a > 1LL) && v_b) {\n"
                "    }\n");
}

void test_enquanto() {
    expect_main("  enquanto a < 10 faca\n    a <- a + 1\n  fimenquanto\n",
                "    while (v_a < 10LL) {\n"
                "        v_a = spike_add_int(v_a, 1LL);\n"
                "    }\n");
}

void test_repita_with_compound_condition() {
    expect_main("  repita\n    a <- a + 1\n  ate (a >= 10) ou b\n",
                "    do {\n"
                "        v_a = spike_add_int(v_a, 1LL);\n"
                "    } while (!((v_a >= 10LL) || v_b));\n");
    expect_main("  repita\n  ate b\n", "    do {\n    } while (!(v_b));\n");
}

void test_para_with_literal_limits() {
    expect_main("  para i de 1 ate 5 faca\n    escreval(i)\n  fimpara\n",
                "    v_i = 1LL;\n"
                "    while (v_i <= 5LL) {\n"
                "        spike_write_int(v_i);\n"
                "        spike_write_newline();\n"
                "        if (!spike_for_advance(&v_i, 1LL)) break;\n"
                "    }\n");
}

void test_para_with_negative_literal_step() {
    expect_main("  para i de 5 ate 1 passo -1 faca\n  fimpara\n",
                "    v_i = 5LL;\n"
                "    while (v_i >= 1LL) {\n"
                "        if (!spike_for_advance(&v_i, (-1LL))) break;\n"
                "    }\n");
}

void test_para_with_variable_end_and_step() {
    expect_main("  para i de 1 ate a * 2 passo a faca\n  fimpara\n",
                "    {\n"
                "        spike_int spike_to_1 = spike_mul_int(v_a, 2LL);\n"
                "        spike_int spike_step_1 = v_a;\n"
                "        spike_check_step(spike_step_1, 8);\n"
                "        v_i = 1LL;\n"
                "        while ((spike_step_1 > 0) ? (v_i <= spike_to_1) : (v_i >= spike_to_1)) {\n"
                "            if (!spike_for_advance(&v_i, spike_step_1)) break;\n"
                "        }\n"
                "    }\n");
    expect_main("  para i de 1 ate a faca\n  fimpara\n",
                "    {\n"
                "        spike_int spike_to_1 = v_a;\n"
                "        v_i = 1LL;\n"
                "        while (v_i <= spike_to_1) {\n"
                "            if (!spike_for_advance(&v_i, 1LL)) break;\n"
                "        }\n"
                "    }\n");
}

void test_nested_structures_get_unique_names() {
    expect_main("  enquanto b faca\n"
                "    para i de 1 ate a faca\n"
                "      se i = 2 entao\n"
                "        para a de i ate a faca\n"
                "        fimpara\n"
                "      fimse\n"
                "    fimpara\n"
                "  fimenquanto\n",
                "    while (v_b) {\n"
                "        {\n"
                "            spike_int spike_to_1 = v_a;\n"
                "            v_i = 1LL;\n"
                "            while (v_i <= spike_to_1) {\n"
                "                if (v_i == 2LL) {\n"
                "                    {\n"
                "                        spike_int spike_to_2 = v_a;\n"
                "                        v_a = v_i;\n"
                "                        while (v_a <= spike_to_2) {\n"
                "                            if (!spike_for_advance(&v_a, 1LL)) break;\n"
                "                        }\n"
                "                    }\n"
                "                }\n"
                "                if (!spike_for_advance(&v_i, 1LL)) break;\n"
                "            }\n"
                "        }\n"
                "    }\n");
}

void test_condition_keeps_text_with_parentheses() {
    expect_main("  se t = \"(\" entao\n  fimse\n",
                "    if (spike_str_eq(v_t, \"(\")) {\n"
                "    }\n");
}

} // namespace

namespace spike_tests {

void run_codegen_control_tests() {
    test_se_with_and_without_senao();
    test_enquanto();
    test_repita_with_compound_condition();
    test_para_with_literal_limits();
    test_para_with_negative_literal_step();
    test_para_with_variable_end_and_step();
    test_nested_structures_get_unique_names();
    test_condition_keeps_text_with_parentheses();
}

} // namespace spike_tests
