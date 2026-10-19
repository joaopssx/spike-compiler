#include "semantic/semantic_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <string>

namespace {

using spike_tests::analyze_typed;
using spike_tests::count_occurrences;

const char* kSubprograms = "procedimento troca(var a: inteiro, var b: inteiro)\n"
                           "var\n"
                           "  t : inteiro\n"
                           "inicio\n"
                           "  t <- a\n"
                           "  a <- b\n"
                           "  b <- t\n"
                           "fimprocedimento\n"
                           "procedimento zera(var r: real)\n"
                           "inicio\n"
                           "  r <- 0\n"
                           "fimprocedimento\n"
                           "funcao proximo(var n: inteiro): inteiro\n"
                           "inicio\n"
                           "  n <- n + 1\n"
                           "  retorne n\n"
                           "fimfuncao\n";

std::string program(const std::string& extra, const std::string& body) {
    return "algoritmo \"t\"\nconst\n  MAX = 3\nvar\n  x, y, i : inteiro\n  r : real\n"
           "  v : vetor[1..3] de inteiro\n  m : vetor[1..2, 1..2] de inteiro\n" +
           std::string(kSubprograms) + extra + "inicio\n" + body + "fimalgoritmo\n";
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

void test_valid_reference_arguments() {
    SPIKE_EXPECT(errors_of("", "  troca(x, y)\n  troca(v[1], v[i + 1])\n  troca(m[1, 2], x)\n"
                               "  troca(x, x)\n  zera(r)\n  y <- proximo(x) * 2\n")
                     .empty());
    SPIKE_EXPECT(errors_of("procedimento repassa(var a: inteiro)\ninicio\n  troca(a, a)\n"
                           "  leia(a)\n  para a de 1 ate 3 faca\n  fimpara\nfimprocedimento\n",
                           "  repassa(y)\n")
                     .empty());
}

void test_reference_argument_counts_as_assigned() {
    auto typed = analyze_typed(program("", "  troca(x, v[2])\n"));
    SPIKE_EXPECT(typed->diag_output.empty());
    const spike::Symbol* x = typed->analyzer->symbols().lookup("x");
    SPIKE_EXPECT(x && x->assigned && x->used);
    SPIKE_EXPECT(typed->analyzer->symbols().lookup("v")->assigned);
    auto loop = analyze_typed(program("", "  para i de 1 ate 3 faca\n    troca(i, x)\n  fimpara\n"));
    SPIKE_EXPECT(loop->diag_output.empty());
    SPIKE_EXPECT(loop->analyzer->loop_var_writes().size() == 1);
}

void test_expression_for_reference_is_e785() {
    const std::string out = errors_of("", "  troca(1, x)\n");
    SPIKE_EXPECT(single_error(out, "E785"));
    SPIKE_EXPECT(contains(out, "o argumento de 'a' precisa ser uma variável, porque o parâmetro é "
                               "por referência"));
    SPIKE_EXPECT(single_error(errors_of("", "  troca(x, y + 1)\n"), "E785"));
    SPIKE_EXPECT(single_error(errors_of("", "  troca(proximo(x), y)\n"), "E785"));
    SPIKE_EXPECT(contains(errors_of("", "  troca(x + nada, y)\n"), "[E700]"));
}

void test_constant_for_reference_is_e786() {
    const std::string out = errors_of("", "  troca(MAX, x)\n");
    SPIKE_EXPECT(single_error(out, "E786"));
    SPIKE_EXPECT(contains(out, "'MAX' é uma constante e não pode ir para o parâmetro 'a', que é "
                               "por referência"));
}

void test_reference_type_must_match_exactly_e787() {
    const std::string out = errors_of("", "  zera(x)\n");
    SPIKE_EXPECT(single_error(out, "E787"));
    SPIKE_EXPECT(contains(out, "o parâmetro 'r' é por referência e exige 'real', mas recebeu "
                               "'inteiro'"));
    SPIKE_EXPECT(single_error(errors_of("", "  troca(r, x)\n"), "E787"));
    SPIKE_EXPECT(errors_of("procedimento mostra(n: real)\ninicio\nfimprocedimento\n",
                           "  mostra(x)\n")
                     .empty());
}

void test_vectors_cannot_be_passed_e788() {
    const std::string declared =
        errors_of("procedimento soma(w: vetor[1..3] de inteiro)\ninicio\n  escreval(w[1])\n"
                  "fimprocedimento\n",
                  "");
    SPIKE_EXPECT(single_error(declared, "E788"));
    SPIKE_EXPECT(contains(declared, "vetores ainda não podem ser passados a procedimentos ou "
                                    "funções"));
    SPIKE_EXPECT(single_error(errors_of("procedimento mostra(n: inteiro)\ninicio\n"
                                        "fimprocedimento\n",
                                        "  mostra(v)\n"),
                              "E788"));
    SPIKE_EXPECT(single_error(errors_of("", "  troca(v, x)\n"), "E788"));
}

void test_argument_count_with_references() {
    SPIKE_EXPECT(single_error(errors_of("", "  troca(x)\n"), "E781"));
    SPIKE_EXPECT(single_error(errors_of("", "  troca(x, y, 3)\n"), "E781"));
}

} // namespace

namespace spike_tests {

void run_semantic_reference_tests() {
    test_valid_reference_arguments();
    test_reference_argument_counts_as_assigned();
    test_expression_for_reference_is_e785();
    test_constant_for_reference_is_e786();
    test_reference_type_must_match_exactly_e787();
    test_vectors_cannot_be_passed_e788();
    test_argument_count_with_references();
}

} // namespace spike_tests
