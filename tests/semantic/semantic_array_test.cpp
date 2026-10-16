#include "semantic/semantic_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <string>

namespace {

using spike_tests::analyze_typed;
using spike_tests::count_occurrences;

std::string errors_of(const std::string& body, const std::string& extra = "") {
    const std::string src = "algoritmo \"t\"\nconst\n  N = 5\nvar\n  v : vetor[1..N] de inteiro\n"
                            "  r : vetor[0..2] de real\n  i : inteiro\n  x : real\n" + extra +
                            "inicio\n" + body + "fimalgoritmo\n";
    return analyze_typed(src)->diag_output;
}

bool single_error(const std::string& out, const std::string& code) {
    return count_occurrences(out, "erro[") == 1 && out.find("[" + code + "]") != std::string::npos;
}

void test_valid_array_use() {
    SPIKE_EXPECT(errors_of("  v[1] <- 2\n  v[i] <- v[i + 1] * 2\n  r[0] <- v[2]\n"
                           "  leia(v[i], r[1])\n  escreval(v[N], r[i])\n  x <- r[1] / 2\n")
                     .empty());
    auto typed = analyze_typed("algoritmo \"t\"\nvar\n  v : vetor[1..3] de caractere\ninicio\n"
                               "  v[1] <- \"a\" + v[2]\nfimalgoritmo\n");
    SPIKE_EXPECT(typed->diag_output.empty());
    SPIKE_EXPECT(typed->analyzer->symbols().lookup("v")->dims.size() == 1);
    SPIKE_EXPECT(typed->analyzer->symbols().lookup("v")->dims[0].high == 3);
}

void test_array_without_index_is_e770() {
    SPIKE_EXPECT(single_error(errors_of("  escreval(v)\n"), "E770"));
    SPIKE_EXPECT(single_error(errors_of("  i <- v + 1\n"), "E770"));
    SPIKE_EXPECT(single_error(errors_of("  v <- 3\n"), "E770"));
    SPIKE_EXPECT(single_error(errors_of("  leia(v)\n"), "E770"));
    SPIKE_EXPECT(errors_of("  escreval(v)\n").find("o vetor 'v' precisa de um índice") !=
                 std::string::npos);
}

void test_index_on_non_array_is_e771() {
    const std::string out = errors_of("  i[1] <- 2\n");
    SPIKE_EXPECT(single_error(out, "E771"));
    SPIKE_EXPECT(out.find("'i' não é um vetor") != std::string::npos);
    SPIKE_EXPECT(single_error(errors_of("  escreval(x[0])\n"), "E771"));
}

void test_non_integer_index_is_e772() {
    SPIKE_EXPECT(single_error(errors_of("  escreval(v[x])\n"), "E772"));
    SPIKE_EXPECT(single_error(errors_of("  v[\"a\"] <- 1\n"), "E772"));
}

void test_wrong_index_count_is_e773() {
    const std::string out = errors_of("  escreval(v[1, 2])\n");
    SPIKE_EXPECT(single_error(out, "E773"));
    SPIKE_EXPECT(out.find("'v' tem 1 dimensão, mas recebeu 2 índices") != std::string::npos);
}

void test_literal_index_out_of_range_is_e774() {
    const std::string out = errors_of("  v[6] <- 1\n");
    SPIKE_EXPECT(single_error(out, "E774"));
    SPIKE_EXPECT(out.find("índice 6 fora do vetor 'v' (válido: 1 a 5)") != std::string::npos);
    SPIKE_EXPECT(single_error(errors_of("  escreval(r[-1])\n"), "E774"));
    SPIKE_EXPECT(single_error(errors_of("  escreval(v[N + 1])\n"), "E774"));
    SPIKE_EXPECT(errors_of("  escreval(v[N])\n").empty());
}

void test_invalid_bounds_are_e775() {
    SPIKE_EXPECT(single_error(errors_of("", "  a : vetor[5..1] de inteiro\n"), "E775"));
    SPIKE_EXPECT(single_error(errors_of("", "  a : vetor[1..20000000] de inteiro\n"), "E775"));
    SPIKE_EXPECT(single_error(errors_of("", "  a : vetor[1..i] de inteiro\n"), "E775"));
    SPIKE_EXPECT(errors_of("", "  a : vetor[5..1] de inteiro\n").find("termina antes de começar") !=
                 std::string::npos);
    SPIKE_EXPECT(errors_of("  escreval(a[1])\n", "  a : vetor[5..1] de inteiro\n")
                     .find("E771") == std::string::npos);
}

void test_array_as_para_counter_is_e731() {
    const std::string out = errors_of("  para v de 1 ate 3 faca\n  fimpara\n");
    SPIKE_EXPECT(single_error(out, "E731"));
    SPIKE_EXPECT(out.find("não pode ser um vetor") != std::string::npos);
}

} // namespace

namespace spike_tests {

void run_semantic_array_tests() {
    test_valid_array_use();
    test_array_without_index_is_e770();
    test_index_on_non_array_is_e771();
    test_non_integer_index_is_e772();
    test_wrong_index_count_is_e773();
    test_literal_index_out_of_range_is_e774();
    test_invalid_bounds_are_e775();
    test_array_as_para_counter_is_e731();
}

} // namespace spike_tests
