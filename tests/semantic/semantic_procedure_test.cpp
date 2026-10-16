#include "semantic/semantic_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/symbol_table.hpp"

#include <string>

namespace {

using spike_tests::analyze_typed;
using spike_tests::count_occurrences;

const char* kSaudar = "procedimento saudar(nome: caractere, vezes: inteiro)\n"
                      "var\n"
                      "  i : inteiro\n"
                      "inicio\n"
                      "  para i de 1 ate vezes faca\n"
                      "    escreval(\"Oi, \", nome)\n"
                      "  fimpara\n"
                      "fimprocedimento\n";

std::string errors_of(const std::string& procedures, const std::string& body) {
    const std::string src = "algoritmo \"t\"\nconst\n  MAX = 3\nvar\n  n, k : inteiro\n  r : real\n"
                            "  t : caractere\n" +
                            procedures + "inicio\n" + body + "fimalgoritmo\n";
    return analyze_typed(src)->diag_output;
}

bool single_error(const std::string& out, const std::string& code) {
    return count_occurrences(out, "erro[") == 1 && out.find("[" + code + "]") != std::string::npos;
}

bool contains(const std::string& out, const std::string& piece) {
    return out.find(piece) != std::string::npos;
}

void test_scope_stack() {
    spike::SymbolTable table;
    SPIKE_EXPECT(table.depth() == 1);
    SPIKE_EXPECT(table.declare("x", spike::TypeKind::Integer, 1, 1));
    table.push_scope();
    SPIKE_EXPECT(table.depth() == 2);
    SPIKE_EXPECT(table.lookup("x")->type == spike::TypeKind::Integer);
    SPIKE_EXPECT(table.declare("X", spike::TypeKind::String, 2, 1));
    SPIKE_EXPECT(!table.declare("x", spike::TypeKind::Real, 3, 1));
    SPIKE_EXPECT(table.lookup("x")->type == spike::TypeKind::String);
    SPIKE_EXPECT(table.lookup_global("x")->type == spike::TypeKind::Integer);
    table.pop_scope();
    SPIKE_EXPECT(table.lookup("x")->type == spike::TypeKind::Integer);
    SPIKE_EXPECT(table.size() == 2);
    table.pop_scope();
    SPIKE_EXPECT(table.depth() == 1);
}

void test_valid_procedures() {
    SPIKE_EXPECT(errors_of(kSaudar, "  saudar(\"Ana\", 2)\n  saudar(t, n + 1)\n").empty());
    SPIKE_EXPECT(errors_of("procedimento antes()\ninicio\n  depois(1)\nfimprocedimento\n"
                           "procedimento depois(a: real)\ninicio\n  escreval(a)\nfimprocedimento\n",
                           "  antes()\n")
                     .empty());
    SPIKE_EXPECT(errors_of("procedimento conta(a: inteiro)\ninicio\n  se a > 0 entao\n"
                           "    conta(a - 1)\n  fimse\nfimprocedimento\n",
                           "  conta(MAX)\n")
                     .empty());
    SPIKE_EXPECT(errors_of("procedimento muda()\ninicio\n  leia(n)\n  k <- n * MAX\nfimprocedimento\n",
                           "  muda()\n")
                     .empty());
}

void test_locals_shadow_globals() {
    auto typed = analyze_typed("algoritmo \"t\"\nvar\n  n : inteiro\nprocedimento p(t: real)\nvar\n"
                               "  n : caractere\ninicio\n  n <- \"local\"\n  t <- 1.5\n"
                               "fimprocedimento\ninicio\n  n <- 2\nfimalgoritmo\n");
    SPIKE_EXPECT(typed->diag_output.empty());
    const spike::Symbol* global = typed->analyzer->symbols().lookup("n");
    SPIKE_EXPECT(global && global->type == spike::TypeKind::Integer && global->line == 3);
    SPIKE_EXPECT(typed->analyzer->symbols().size() == 3);
    SPIKE_EXPECT(errors_of("procedimento p(n: caractere)\ninicio\n  n <- \"x\"\nfimprocedimento\n",
                           "  n <- 1\n")
                     .empty());
    SPIKE_EXPECT(errors_of("procedimento p(MAX: real)\ninicio\n  MAX <- 2.5\nfimprocedimento\n", "")
                     .empty());
}

void test_locals_are_not_visible_outside() {
    const std::string out = errors_of(kSaudar, "  i <- 1\n");
    SPIKE_EXPECT(single_error(out, "E700"));
    SPIKE_EXPECT(single_error(errors_of(kSaudar + std::string("procedimento outro()\ninicio\n"
                                                              "  escreval(vezes)\n"
                                                              "fimprocedimento\n"),
                                        ""),
                              "E700"));
}

void test_duplicate_names_in_one_scope_are_e701() {
    SPIKE_EXPECT(single_error(errors_of("procedimento p(a: inteiro, A: real)\ninicio\n"
                                        "fimprocedimento\n",
                                        ""),
                              "E701"));
    SPIKE_EXPECT(contains(errors_of("procedimento p(a: inteiro, a: real)\ninicio\n"
                                    "fimprocedimento\n",
                                    ""),
                          "parâmetro 'a' repetido"));
    SPIKE_EXPECT(single_error(errors_of("procedimento p(a: inteiro)\nvar\n  a : real\ninicio\n"
                                        "fimprocedimento\n",
                                        ""),
                              "E701"));
    SPIKE_EXPECT(single_error(errors_of("procedimento p()\nvar\n  b, b : real\ninicio\n"
                                        "fimprocedimento\n",
                                        ""),
                              "E701"));
}

void test_undefined_procedure_is_e780() {
    const std::string out = errors_of("", "  saudar(\"Ana\")\n");
    SPIKE_EXPECT(single_error(out, "E780"));
    SPIKE_EXPECT(contains(out, "procedimento 'saudar' não foi definido"));
    SPIKE_EXPECT(contains(errors_of("", "  n(1)\n"), "'n' é uma variável"));
    SPIKE_EXPECT(contains(errors_of("", "  n(k + nada)\n"), "[E700]"));
}

void test_argument_count_is_e781() {
    const std::string out = errors_of(kSaudar, "  saudar(\"Ana\")\n");
    SPIKE_EXPECT(single_error(out, "E781"));
    SPIKE_EXPECT(contains(out, "'saudar' espera 2 argumentos, recebeu 1"));
    SPIKE_EXPECT(contains(errors_of("procedimento p()\ninicio\nfimprocedimento\n", "  p(1)\n"),
                          "'p' espera 0 argumentos, recebeu 1"));
    SPIKE_EXPECT(contains(errors_of("procedimento p(a: inteiro)\ninicio\nfimprocedimento\n",
                                    "  p()\n"),
                          "'p' espera 1 argumento, recebeu 0"));
}

void test_argument_type_is_e782() {
    const std::string out = errors_of(kSaudar, "  saudar(3, 2)\n");
    SPIKE_EXPECT(single_error(out, "E782"));
    SPIKE_EXPECT(contains(out, "o argumento 1 de 'saudar' precisa ser 'caractere', mas é 'inteiro'"));
    const std::string real_to_int = errors_of(kSaudar, "  saudar(t, r)\n");
    SPIKE_EXPECT(single_error(real_to_int, "E782"));
    SPIKE_EXPECT(contains(real_to_int, "trunca()"));
    SPIKE_EXPECT(errors_of("procedimento p(a: real, b: logico)\ninicio\nfimprocedimento\n",
                           "  p(k, n > 1)\n")
                     .empty());
    SPIKE_EXPECT(single_error(errors_of(kSaudar, "  saudar(t, 1 + \"a\")\n"), "E710"));
}

void test_duplicate_procedure_is_e783() {
    const std::string out = errors_of("procedimento p()\ninicio\nfimprocedimento\n"
                                      "procedimento P()\ninicio\nfimprocedimento\n",
                                      "  p()\n");
    SPIKE_EXPECT(single_error(out, "E783"));
    SPIKE_EXPECT(contains(out, "procedimento 'P' já foi declarado"));
    SPIKE_EXPECT(contains(errors_of("procedimento k()\ninicio\nfimprocedimento\n", ""),
                          "'k' já foi declarada como variável"));
    SPIKE_EXPECT(contains(errors_of("procedimento MAX()\ninicio\nfimprocedimento\n", ""),
                          "'MAX' já foi declarada como constante"));
}

void test_procedure_as_value_is_e784() {
    const std::string out = errors_of(kSaudar, "  n <- saudar\n");
    SPIKE_EXPECT(single_error(out, "E784"));
    SPIKE_EXPECT(contains(out, "'saudar' é um procedimento, não uma variável"));
    SPIKE_EXPECT(single_error(errors_of(kSaudar, "  saudar <- 1\n"), "E784"));
    SPIKE_EXPECT(single_error(errors_of(kSaudar, "  leia(saudar)\n"), "E784"));
}

void test_interrompa_needs_a_loop_inside_the_procedure() {
    SPIKE_EXPECT(single_error(errors_of("procedimento p()\ninicio\n  interrompa\nfimprocedimento\n",
                                        "  enquanto n < 3 faca\n    p()\n  fimenquanto\n"),
                              "E750"));
    SPIKE_EXPECT(errors_of("procedimento p()\ninicio\n  enquanto n < 3 faca\n    interrompa\n"
                           "  fimenquanto\nfimprocedimento\n",
                           "")
                     .empty());
}

void test_local_vector_limit() {
    SPIKE_EXPECT(errors_of("procedimento p()\nvar\n  v : vetor[1..10000] de inteiro\n"
                           "  w : vetor[1..MAX, 1..MAX] de real\ninicio\n  v[1] <- 1\n"
                           "  w[MAX, 1] <- 2\nfimprocedimento\n",
                           "")
                     .empty());
    const std::string out = errors_of("procedimento p()\nvar\n  v : vetor[0..10000] de inteiro\n"
                                      "inicio\nfimprocedimento\n",
                                      "");
    SPIKE_EXPECT(single_error(out, "E775"));
    SPIKE_EXPECT(contains(out, "o vetor local 'v' é grande demais (o limite é 10000 elementos)"));
}

} // namespace

namespace spike_tests {

void run_semantic_procedure_tests() {
    test_scope_stack();
    test_valid_procedures();
    test_locals_shadow_globals();
    test_locals_are_not_visible_outside();
    test_duplicate_names_in_one_scope_are_e701();
    test_undefined_procedure_is_e780();
    test_argument_count_is_e781();
    test_argument_type_is_e782();
    test_duplicate_procedure_is_e783();
    test_procedure_as_value_is_e784();
    test_interrompa_needs_a_loop_inside_the_procedure();
    test_local_vector_limit();
}

} // namespace spike_tests
