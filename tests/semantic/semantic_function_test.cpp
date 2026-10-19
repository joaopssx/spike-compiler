#include "semantic/semantic_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <string>
#include <variant>

namespace {

using spike_tests::analyze_typed;
using spike_tests::count_occurrences;

const char* kFunctions = "funcao dobro(n: inteiro): inteiro\n"
                         "inicio\n"
                         "  retorne n * 2\n"
                         "fimfuncao\n"
                         "funcao metade(x: real): real\n"
                         "inicio\n"
                         "  retorne x / 2\n"
                         "fimfuncao\n"
                         "funcao par(n: inteiro): logico\n"
                         "inicio\n"
                         "  retorne n mod 2 = 0\n"
                         "fimfuncao\n"
                         "procedimento mostra(t: caractere)\n"
                         "inicio\n"
                         "  escreval(t)\n"
                         "fimprocedimento\n";

std::string program(const std::string& extra, const std::string& body) {
    return "algoritmo \"t\"\nvar\n  i : inteiro\n  r : real\n  t : caractere\n" +
           std::string(kFunctions) + extra + "inicio\n" + body + "fimalgoritmo\n";
}

std::string errors_of(const std::string& extra, const std::string& body) {
    return analyze_typed(program(extra, body))->diag_output;
}

bool single_error(const std::string& out, const std::string& code) {
    return count_occurrences(out, "erro[") == 1 && out.find("[" + code + "]") != std::string::npos;
}

bool contains(const std::string& out, const std::string& piece) {
    return out.find(piece) != std::string::npos;
}

void test_valid_function_use() {
    SPIKE_EXPECT(errors_of("", "  i <- dobro(dobro(3)) + 1\n  r <- metade(i)\n"
                               "  se par(i) e nao par(dobro(i) + 1) entao\n    mostra(\"par\")\n"
                               "  fimse\n  escreval(metade(dobro(i)))\n  dobro(4)\n")
                     .empty());
    SPIKE_EXPECT(errors_of("funcao fat(n: inteiro): inteiro\ninicio\n  se n <= 1 entao\n"
                           "    retorne 1\n  fimse\n  retorne n * fat(n - 1)\nfimfuncao\n",
                           "  i <- fat(5)\n")
                     .empty());
    SPIKE_EXPECT(errors_of("funcao primeiro(): inteiro\ninicio\n  retorne segundo()\nfimfuncao\n"
                           "funcao segundo(): inteiro\ninicio\n  retorne 2\nfimfuncao\n",
                           "")
                     .empty());
}

void test_call_type_is_recorded() {
    auto typed = analyze_typed(program("", "  escreval(metade(3), par(2))\n"));
    SPIKE_EXPECT(typed->diag_output.empty());
    const auto& escreva = std::get<spike::EscrevaStmt>(typed->program.body[0]->node);
    const auto& types = typed->analyzer->expr_types();
    SPIKE_EXPECT(types.at(escreva.args[0].get()) == spike::TypeKind::Real);
    SPIKE_EXPECT(types.at(escreva.args[1].get()) == spike::TypeKind::Bool);
}

void test_call_errors_inside_expressions() {
    const std::string undefined = errors_of("", "  i <- triplo(2)\n");
    SPIKE_EXPECT(single_error(undefined, "E780"));
    SPIKE_EXPECT(contains(undefined, "função 'triplo' não foi definida"));
    SPIKE_EXPECT(contains(errors_of("", "  i <- i(2)\n"), "'i' é uma variável"));
    SPIKE_EXPECT(contains(errors_of("", "  i <- dobro()\n"), "'dobro' espera 1 argumento, recebeu 0"));
    SPIKE_EXPECT(single_error(errors_of("", "  i <- dobro(r)\n"), "E782"));
    SPIKE_EXPECT(single_error(errors_of("", "  i <- dobro(1, 2) + 1\n"), "E781"));
    SPIKE_EXPECT(single_error(errors_of("", "  t <- dobro(1)\n"), "E720"));
}

void test_procedure_in_expression_is_e784() {
    const std::string out = errors_of("", "  i <- mostra(\"a\")\n");
    SPIKE_EXPECT(single_error(out, "E784"));
    SPIKE_EXPECT(contains(out, "'mostra' é um procedimento e não devolve valor"));
    const std::string as_value = errors_of("", "  i <- dobro\n");
    SPIKE_EXPECT(single_error(as_value, "E784"));
    SPIKE_EXPECT(contains(as_value, "'dobro' é uma função, não uma variável"));
}

void test_functions_share_names_with_procedures() {
    const std::string out = errors_of("procedimento dobro()\ninicio\nfimprocedimento\n", "");
    SPIKE_EXPECT(single_error(out, "E783"));
    SPIKE_EXPECT(contains(out, "função 'dobro' já foi declarada"));
    SPIKE_EXPECT(single_error(errors_of("funcao i(): inteiro\ninicio\n  retorne 1\nfimfuncao\n", ""),
                              "E783"));
}

void test_return_type_mismatch_is_e790() {
    const std::string out = errors_of("funcao f(): inteiro\ninicio\n  retorne \"um\"\nfimfuncao\n", "");
    SPIKE_EXPECT(single_error(out, "E790"));
    SPIKE_EXPECT(contains(out, "a função 'f' devolve 'inteiro', mas o valor é 'caractere'"));
    SPIKE_EXPECT(contains(errors_of("funcao f(): inteiro\ninicio\n  retorne 1.5\nfimfuncao\n", ""),
                          "trunca()"));
    SPIKE_EXPECT(errors_of("funcao f(): real\ninicio\n  retorne 1\nfimfuncao\n", "").empty());
    SPIKE_EXPECT(single_error(errors_of("funcao f(): inteiro\ninicio\n  retorne 1 + \"a\"\n"
                                        "fimfuncao\n",
                                        ""),
                              "E710"));
}

void test_retorne_outside_subprogram_is_e791() {
    const std::string out = errors_of("", "  retorne\n");
    SPIKE_EXPECT(single_error(out, "E791"));
    SPIKE_EXPECT(single_error(errors_of("", "  retorne 1\n"), "E791"));
}

void test_retorne_value_must_match_kind_e793() {
    const std::string missing = errors_of("funcao f(): inteiro\ninicio\n  retorne\nfimfuncao\n", "");
    SPIKE_EXPECT(single_error(missing, "E793"));
    SPIKE_EXPECT(contains(missing, "a função 'f' precisa devolver um valor"));
    const std::string extra = errors_of("procedimento p()\ninicio\n  retorne 1\nfimprocedimento\n",
                                        "");
    SPIKE_EXPECT(single_error(extra, "E793"));
    SPIKE_EXPECT(contains(extra, "o procedimento 'p' não devolve valor"));
    SPIKE_EXPECT(errors_of("procedimento p()\ninicio\n  se i > 0 entao\n    retorne\n  fimse\n"
                           "  escreval(i)\nfimprocedimento\n",
                           "")
                     .empty());
}

void test_functions_without_return_are_listed() {
    auto typed = analyze_typed(program(
        "funcao a(n: inteiro): inteiro\ninicio\n  se n > 0 entao\n    retorne 1\n  fimse\n"
        "fimfuncao\n"
        "funcao b(n: inteiro): inteiro\ninicio\n  se n > 0 entao\n    retorne 1\n  senao\n"
        "    retorne 2\n  fimse\nfimfuncao\n"
        "funcao c(n: inteiro): inteiro\ninicio\n  enquanto n > 0 faca\n    retorne n\n"
        "  fimenquanto\nfimfuncao\n"
        "funcao d(n: inteiro): inteiro\ninicio\n  escolha n\n  caso 1\n    retorne 1\n"
        "  outrocaso\n    retorne 0\n  fimescolha\nfimfuncao\n",
        ""));
    SPIKE_EXPECT(typed->diag_output.empty());
    const auto& missing = typed->analyzer->functions_without_return();
    SPIKE_EXPECT(missing.size() == 2);
    if (missing.size() == 2) SPIKE_EXPECT(missing[0].name == "a" && missing[1].name == "c");
}

void test_calls_are_never_constant() {
    auto out = analyze_typed("algoritmo \"t\"\nconst\n  X = dobro(2)\nfuncao dobro(n: inteiro): "
                             "inteiro\ninicio\n  retorne n * 2\nfimfuncao\ninicio\nfimalgoritmo\n")
                   ->diag_output;
    SPIKE_EXPECT(single_error(out, "E741"));
    SPIKE_EXPECT(single_error(errors_of("", "  escolha i\n  caso dobro(1)\n    escreval(1)\n"
                                            "  fimescolha\n"),
                              "E763"));
}

void test_interrompa_in_function_needs_its_own_loop() {
    SPIKE_EXPECT(single_error(errors_of("funcao f(): inteiro\ninicio\n  interrompa\n"
                                        "  retorne 1\nfimfuncao\n",
                                        ""),
                              "E750"));
}

} // namespace

namespace spike_tests {

void run_semantic_function_tests() {
    test_valid_function_use();
    test_call_type_is_recorded();
    test_call_errors_inside_expressions();
    test_procedure_in_expression_is_e784();
    test_functions_share_names_with_procedures();
    test_return_type_mismatch_is_e790();
    test_retorne_outside_subprogram_is_e791();
    test_retorne_value_must_match_kind_e793();
    test_functions_without_return_are_listed();
    test_calls_are_never_constant();
    test_interrompa_in_function_needs_its_own_loop();
}

} // namespace spike_tests
