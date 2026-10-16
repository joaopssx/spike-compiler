#include "codegen/codegen_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "codegen/codegen_internal.hpp"

#include <climits>
#include <string>

namespace {

using spike_tests::contains;
using spike_tests::generate_from_source;

void test_constants_come_before_variables_with_folded_values() {
    const auto gen = generate_from_source(
        "algoritmo \"t\"\nconst\n  PI = 3.14159\n  N = 2 * 5\n  NOME = \"Á\" + \"rea\"\n"
        "  OK = N > 3\n  NEG = -7\nvar\n  r : real\ninicio\n  r <- PI * N\nfimalgoritmo\n");
    SPIKE_EXPECT(gen.result.ok);
    const std::string& c = gen.result.c_source;
    SPIKE_EXPECT(contains(c, "const spike_real v_pi = 3.14159;\n"
                             "const spike_int v_n = 10LL;\n"
                             "const spike_str v_nome = \"Área\";\n"
                             "const spike_bool v_ok = 1;\n"
                             "const spike_int v_neg = -7LL;\n"
                             "\n"
                             "spike_real v_r = 0.0;\n"));
    SPIKE_EXPECT(contains(c, "    v_r = (v_pi * ((spike_real)v_n));\n"));
}

void test_program_without_constants_has_no_const_lines() {
    const auto gen = generate_from_source("algoritmo \"t\"\ninicio\nfimalgoritmo\n");
    SPIKE_EXPECT(!contains(gen.result.c_source, "\nconst spike_"));
}

void test_integer_literal_edges() {
    using spike::codegen_detail::integer_literal;
    SPIKE_EXPECT(integer_literal(0) == "0LL");
    SPIKE_EXPECT(integer_literal(-42) == "-42LL");
    SPIKE_EXPECT(integer_literal(LLONG_MAX) == "9223372036854775807LL");
    SPIKE_EXPECT(integer_literal(LLONG_MIN) == "(-9223372036854775807LL - 1)");
}

} // namespace

namespace spike_tests {

void run_codegen_const_tests() {
    test_constants_come_before_variables_with_folded_values();
    test_program_without_constants_has_no_const_lines();
    test_integer_literal_edges();
}

} // namespace spike_tests
