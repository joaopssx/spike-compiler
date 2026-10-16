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

std::string generate_matrix(const std::string& body) {
    return generate_from_source("algoritmo \"t\"\nvar\n  m : vetor[1..3, 1..4] de inteiro\n"
                                "  c : vetor[0..1, -1..1, 1..2] de real\n  i : inteiro\ninicio\n" +
                                body + "fimalgoritmo\n")
        .result.c_source;
}

void test_matrix_storage_is_linear() {
    expect(generate_matrix(""), "spike_int v_m[12];\nspike_real v_c[12];\n");
}

void test_matrix_offset_is_row_major() {
    expect(generate_matrix("  m[i, 2] <- 7\n"),
           "    v_m[(spike_index_dim(v_i, 1LL, 3LL, 7, \"m\", 1) * 4) + "
           "spike_index_dim(2LL, 1LL, 4LL, 7, \"m\", 2)] = 7LL;\n");
    expect(generate_matrix("  escreval(c[1, i, 2])\n"),
           "v_c[(((spike_index_dim(1LL, 0LL, 1LL, 7, \"c\", 1) * 3) + "
           "spike_index_dim(v_i, -1LL, 1LL, 7, \"c\", 2)) * 2) + "
           "spike_index_dim(2LL, 1LL, 2LL, 7, \"c\", 3)]");
}

} // namespace

namespace spike_tests {

void run_codegen_array_tests() {
    test_array_globals_and_text_fill();
    test_element_access_is_checked();
    test_negative_lower_bound_and_conversion();
    test_leia_into_element();
    test_matrix_storage_is_linear();
    test_matrix_offset_is_row_major();
}

} // namespace spike_tests
