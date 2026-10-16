#include "semantic/semantic_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <string>

namespace {

using spike_tests::analyze_typed;
using spike_tests::count_occurrences;

std::string errors_of(const std::string& body) {
    const std::string src =
        "algoritmo \"t\"\nconst\n  TRES = 3\n  NOME = \"ana\"\nvar\n  i : inteiro\n  r : real\n"
        "  t : caractere\n  b : logico\ninicio\n" + body + "fimalgoritmo\n";
    return analyze_typed(src)->diag_output;
}

bool single_error(const std::string& out, const std::string& code) {
    return count_occurrences(out, "erro[") == 1 && out.find("[" + code + "]") != std::string::npos;
}

void test_valid_escolha() {
    SPIKE_EXPECT(errors_of("  escolha i\n    caso 1\n    caso 2, TRES\n    caso 4 ate 6\n"
                           "    outrocaso\n  fimescolha\n")
                     .empty());
    SPIKE_EXPECT(errors_of("  escolha t + \"!\"\n    caso \"a\", NOME\n  fimescolha\n").empty());
    SPIKE_EXPECT(errors_of("  escolha i div 2\n    caso -1 ate 1\n  fimescolha\n").empty());
}

void test_selector_type_is_e760() {
    SPIKE_EXPECT(single_error(errors_of("  escolha r\n    caso 1\n  fimescolha\n"), "E760"));
    const std::string out = errors_of("  escolha b\n    caso verdadeiro\n  fimescolha\n");
    SPIKE_EXPECT(single_error(out, "E760"));
    SPIKE_EXPECT(out.find("'escolha' aceita inteiro ou caractere, mas o valor é 'logico'") !=
                 std::string::npos);
}

void test_case_type_mismatch_is_e761() {
    const std::string out = errors_of("  escolha i\n    caso \"um\"\n  fimescolha\n");
    SPIKE_EXPECT(single_error(out, "E761"));
    SPIKE_EXPECT(out.find("o caso tem tipo 'caractere', mas o 'escolha' é de 'inteiro'") !=
                 std::string::npos);
    SPIKE_EXPECT(single_error(errors_of("  escolha t\n    caso 1.5\n  fimescolha\n"), "E761"));
}

void test_repeated_value_is_e762() {
    const std::string out = errors_of("  escolha i\n    caso 1, 3\n    caso TRES\n  fimescolha\n");
    SPIKE_EXPECT(single_error(out, "E762"));
    SPIKE_EXPECT(out.find("o valor 3 aparece em dois casos") != std::string::npos);
    const std::string range = errors_of("  escolha i\n    caso 1 ate 5\n    caso 4 ate 9\n  fimescolha\n");
    SPIKE_EXPECT(range.find("os valores de 4 a 5 aparecem em dois casos") != std::string::npos);
    SPIKE_EXPECT(single_error(errors_of("  escolha t\n    caso \"ana\"\n    caso NOME\n  fimescolha\n"),
                              "E762"));
}

void test_variable_as_case_is_e763() {
    SPIKE_EXPECT(single_error(errors_of("  escolha i\n    caso i\n  fimescolha\n"), "E763"));
    SPIKE_EXPECT(single_error(errors_of("  escolha i\n    caso 1 ate i\n  fimescolha\n"), "E763"));
    SPIKE_EXPECT(single_error(errors_of("  escolha i\n    caso zzz\n  fimescolha\n"), "E700"));
}

void test_reversed_range_is_e764() {
    const std::string out = errors_of("  escolha i\n    caso 6 ate 4\n  fimescolha\n");
    SPIKE_EXPECT(single_error(out, "E764"));
    SPIKE_EXPECT(out.find("o intervalo começa em 6 e termina em 4") != std::string::npos);
    SPIKE_EXPECT(out.find("caso 4 ate 6") != std::string::npos);
}

void test_text_range_is_e765() {
    SPIKE_EXPECT(single_error(errors_of("  escolha t\n    caso \"a\" ate \"m\"\n  fimescolha\n"),
                              "E765"));
}

void test_bodies_are_checked_and_break_follows_loops() {
    SPIKE_EXPECT(single_error(errors_of("  escolha i\n    caso 1\n      i <- \"x\"\n"
                                        "    outrocaso\n  fimescolha\n"),
                              "E720"));
    SPIKE_EXPECT(errors_of("  enquanto b faca\n    escolha i\n      caso 1\n        interrompa\n"
                           "    fimescolha\n  fimenquanto\n")
                     .empty());
    SPIKE_EXPECT(single_error(errors_of("  escolha i\n    caso 1\n      interrompa\n  fimescolha\n"),
                              "E750"));
}

} // namespace

namespace spike_tests {

void run_semantic_escolha_tests() {
    test_valid_escolha();
    test_selector_type_is_e760();
    test_case_type_mismatch_is_e761();
    test_repeated_value_is_e762();
    test_variable_as_case_is_e763();
    test_reversed_range_is_e764();
    test_text_range_is_e765();
    test_bodies_are_checked_and_break_follows_loops();
}

} // namespace spike_tests
