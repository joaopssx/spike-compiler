#include "semantic/semantic_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/symbol_table.hpp"

#include <string>

namespace {

using spike::TypeKind;
using spike_tests::analyze_typed;
using spike_tests::count_occurrences;

std::string program(const std::string& consts, const std::string& body) {
    return "algoritmo \"t\"\nconst\n" + consts + "var\n  i : inteiro\n  r : real\ninicio\n" + body +
           "fimalgoritmo\n";
}

bool single_error(const std::string& out, const std::string& code) {
    return count_occurrences(out, "erro[") == 1 && out.find("[" + code + "]") != std::string::npos;
}

void test_constant_types_and_values() {
    auto typed = analyze_typed(program(
        "  PI = 3.5\n  N = 10\n  NOME = \"Area\" + \"!\"\n  OK = N > 5\n  DOBRO = N * 2\n"
        "  METADE = N / 4\n  NEG = -N mod 3\n  POT = 2 ^ 10\n",
        ""));
    SPIKE_EXPECT(typed->diag_output.empty());
    const auto& symbols = typed->analyzer->symbols();
    SPIKE_EXPECT(symbols.lookup("pi")->type == TypeKind::Real);
    SPIKE_EXPECT(symbols.lookup("pi")->is_const);
    SPIKE_EXPECT(symbols.lookup("n")->type == TypeKind::Integer);
    SPIKE_EXPECT(symbols.lookup("nome")->type == TypeKind::String);
    SPIKE_EXPECT(symbols.lookup("ok")->type == TypeKind::Bool);
    SPIKE_EXPECT(!symbols.lookup("i")->is_const);

    const auto& values = typed->analyzer->constants();
    SPIKE_EXPECT(values.at("dobro").integer == 20);
    SPIKE_EXPECT(values.at("metade").type == TypeKind::Real && values.at("metade").real == 2.5);
    SPIKE_EXPECT(values.at("nome").text == "Area!");
    SPIKE_EXPECT(values.at("ok").logical);
    SPIKE_EXPECT(values.at("neg").integer == -1);
    SPIKE_EXPECT(values.at("pot").real == 1024.0);
}

void test_constants_in_expressions() {
    auto typed = analyze_typed(program("  N = 3\n  PI = 3.14\n",
                                       "  para i de 1 ate N faca\n  fimpara\n  r <- PI * 2\n"
                                       "  i <- N div 2\n"));
    SPIKE_EXPECT(typed->diag_output.empty());
    auto bad = analyze_typed(program("  PI = 3.14\n", "  i <- PI\n"));
    SPIKE_EXPECT(single_error(bad->diag_output, "E721"));
}

void test_writing_a_constant_is_e740() {
    for (const std::string body : {"  N <- 2\n", "  leia(N)\n", "  para N de 1 ate 3 faca\n  fimpara\n"}) {
        const std::string out = analyze_typed(program("  N = 1\n", body))->diag_output;
        SPIKE_EXPECT(single_error(out, "E740"));
        SPIKE_EXPECT(out.find("não é possível alterar a constante 'N'") != std::string::npos);
        SPIKE_EXPECT(out.find("declare 'N' na seção var") != std::string::npos);
    }
    const std::string out = analyze_typed(program("  N = 1\n", "  n <- 2\n"))->diag_output;
    SPIKE_EXPECT(out.find("constante 'N'") != std::string::npos);
}

void test_non_constant_value_is_e741() {
    const std::string variable = analyze_typed(program("  N = i + 1\n", ""))->diag_output;
    SPIKE_EXPECT(single_error(variable, "E741"));
    SPIKE_EXPECT(variable.find("'i' não é uma constante declarada antes de 'N'") != std::string::npos);

    const std::string later = analyze_typed(program("  A = B\n  B = 1\n", ""))->diag_output;
    SPIKE_EXPECT(single_error(later, "E741"));

    const std::string unknown = analyze_typed(program("  A = zzz\n", ""))->diag_output;
    SPIKE_EXPECT(single_error(unknown, "E700"));
}

void test_impossible_value_is_e742() {
    for (const std::string value : {"1 div 0", "1 mod 0", "1 / 0", "0 ^ -1"}) {
        const std::string out = analyze_typed(program("  X = " + value + "\n", ""))->diag_output;
        SPIKE_EXPECT(single_error(out, "E742"));
    }
    const std::string out = analyze_typed(program("  X = 1 div 0\n", ""))->diag_output;
    SPIKE_EXPECT(out.find("não foi possível calcular a constante 'X': divisão por zero") !=
                 std::string::npos);
}

void test_constant_type_errors_do_not_cascade() {
    const std::string out =
        analyze_typed(program("  X = \"a\" + 1\n", "  r <- X\n"))->diag_output;
    SPIKE_EXPECT(single_error(out, "E710"));
}

void test_duplicate_names_are_e701() {
    const std::string twice = analyze_typed(program("  N = 1\n  n = 2\n", ""))->diag_output;
    SPIKE_EXPECT(single_error(twice, "E701"));
    const std::string clash = analyze_typed("algoritmo \"t\"\nconst\n  PI = 3\nvar\n  pi : real\n"
                                            "inicio\nfimalgoritmo\n")
                                  ->diag_output;
    SPIKE_EXPECT(single_error(clash, "E701"));
    SPIKE_EXPECT(clash.find("'pi' já foi declarada como constante") != std::string::npos);
}

} // namespace

namespace spike_tests {

void run_semantic_const_tests() {
    test_constant_types_and_values();
    test_constants_in_expressions();
    test_writing_a_constant_is_e740();
    test_non_constant_value_is_e741();
    test_impossible_value_is_e742();
    test_constant_type_errors_do_not_cascade();
    test_duplicate_names_are_e701();
}

} // namespace spike_tests
