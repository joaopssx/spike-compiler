#include "codegen/codegen_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <iostream>
#include <string>

namespace {

using spike_tests::contains;
using spike_tests::generate_from_source;

std::string generate(const std::string& body) {
    return generate_from_source("algoritmo \"t\"\nconst\n  N = 4\nvar\n"
                                "  v : vetor[1..N] de inteiro\n  z : vetor[-2..2] de real\n"
                                "  nomes : vetor[0..2] de caractere\n  i : inteiro\ninicio\n" +
                                body + "fimalgoritmo\n")
        .result.c_source;
}

void expect(const std::string& c, const std::string& piece) {
    const bool ok = contains(c, piece);
    if (!ok) std::cerr << "  expected C:\n" << piece;
    SPIKE_EXPECT(ok);
}

void test_array_globals_and_text_fill() {
    const std::string c = generate("");
    expect(c, "spike_int v_v[4];\nspike_real v_z[5];\nspike_str v_nomes[3];\nspike_int v_i = 0LL;\n");
    expect(c, "    spike_init();\n    spike_fill_str(v_nomes, 3);\n");
}

void test_element_access_is_checked() {
    const std::string c = generate("  v[i + 1] <- v[2] * 3\n");
    expect(c, "    v_v[spike_index(spike_add_int(v_i, 1LL), 1LL, 4LL, 10, \"v\")] = "
              "spike_mul_int(v_v[spike_index(2LL, 1LL, 4LL, 10, \"v\")], 3LL);\n");
}

void test_negative_lower_bound_and_conversion() {
    const std::string c = generate("  z[-2] <- i\n");
    expect(c, "    v_z[spike_index((-2LL), -2LL, 2LL, 10, \"z\")] = ((spike_real)v_i);\n");
}

void test_leia_into_element() {
    const std::string c = generate("  leia(nomes[i])\n");
    expect(c, "    v_nomes[spike_index(v_i, 0LL, 2LL, 10, \"nomes\")] = spike_read_str(10, \"nomes\");\n");
}

} // namespace

namespace spike_tests {

void run_codegen_array_tests() {
    test_array_globals_and_text_fill();
    test_element_access_is_checked();
    test_negative_lower_bound_and_conversion();
    test_leia_into_element();
}

} // namespace spike_tests
