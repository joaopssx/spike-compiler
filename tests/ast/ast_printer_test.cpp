#include "parser/parser_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "driver/pipeline_internal.hpp"
#include "spike/ast_printer.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace {

using spike_tests::parse_source;

const char* kProgram =
    "algoritmo \"soma\"\n"
    "var\n"
    "  a, b : inteiro\n"
    "  t : caractere\n"
    "inicio\n"
    "  a <- 10 + b * -2\n"
    "  se nao (a > 1) entao\n"
    "    escreval(\"x\", 1.5)\n"
    "  senao\n"
    "    leia(a, b)\n"
    "  fimse\n"
    "  para a de 1 ate 3 passo 1 faca\n"
    "    enquanto verdadeiro faca\n"
    "      t <- \"oi\\n\"\n"
    "    fimenquanto\n"
    "  fimpara\n"
    "  repita\n"
    "    escreva()\n"
    "  ate a = 0\n"
    "fimalgoritmo\n";

const char* kExpectedTree =
    "Program \"soma\" (1:1)\n"
    "  Var a : inteiro (3:3)\n"
    "  Var b : inteiro (3:6)\n"
    "  Var t : caractere (4:3)\n"
    "  Assign a (6:3)\n"
    "    Binary + (6:11)\n"
    "      Number 10\n"
    "      Binary * (6:15)\n"
    "        Var b\n"
    "        Unary - (6:17)\n"
    "          Number 2\n"
    "  Se (7:3)\n"
    "    Unary nao (7:6)\n"
    "      Binary > (7:13)\n"
    "        Var a\n"
    "        Number 1\n"
    "    entao\n"
    "      Escreval (8:5)\n"
    "        String \"x\"\n"
    "        Number 1.5\n"
    "    senao\n"
    "      Leia a, b (10:5)\n"
    "  Para a (12:3)\n"
    "    de\n"
    "      Number 1\n"
    "    ate\n"
    "      Number 3\n"
    "    passo\n"
    "      Number 1\n"
    "    faca\n"
    "      Enquanto (13:5)\n"
    "        Bool verdadeiro\n"
    "        faca\n"
    "          Assign t (14:7)\n"
    "            String \"oi\\n\"\n"
    "  Repita (17:3)\n"
    "    corpo\n"
    "      Escreva (18:5)\n"
    "    ate\n"
    "      Binary = (19:9)\n"
    "        Var a\n"
    "        Number 0\n";

void test_print_ast_covers_every_node() {
    auto r = parse_source(kProgram);
    SPIKE_EXPECT(!r.had_error);
    const std::string tree = spike::print_ast(r.program);
    if (tree != kExpectedTree) std::cerr << "  got:\n" << tree;
    SPIKE_EXPECT(tree == kExpectedTree);
}

void test_print_ast_minimal_program() {
    auto r = parse_source("algoritmo \"vazio\"\ninicio\nfimalgoritmo\n");
    SPIKE_EXPECT(spike::print_ast(r.program) == "Program \"vazio\" (1:1)\n");
}

void test_count_statements_is_recursive() {
    auto r = parse_source(kProgram);
    SPIKE_EXPECT(spike::count_statements(r.program.body) == 9);
}

void test_count_text_plural() {
    using spike::pipeline_detail::count_text;
    SPIKE_EXPECT(count_text(1, "variável", "variáveis") == "1 variável");
    SPIKE_EXPECT(count_text(0, "variável", "variáveis") == "0 variáveis");
    SPIKE_EXPECT(count_text(7, "comando", "comandos") == "7 comandos");
}

void test_time_table_is_aligned() {
    using spike::pipeline_detail::PhaseTime;
    const std::vector<PhaseTime> phases = {
        {"léxico", 0.25}, {"geração de C", 12.0}, {"compilador C", 410.5}};
    const std::string expected =
        "tempo por etapa:\n"
        "  léxico            0.2 ms\n"
        "  geração de C     12.0 ms\n"
        "  compilador C    410.5 ms\n"
        "  total           423.0 ms\n";
    const std::string table = spike::pipeline_detail::format_time_table(phases, 423.0);
    if (table != expected) std::cerr << "  got:\n" << table;
    SPIKE_EXPECT(table == expected);
}

} // namespace

namespace spike_tests {

void run_ast_printer_tests() {
    test_print_ast_covers_every_node();
    test_print_ast_minimal_program();
    test_count_statements_is_recursive();
    test_count_text_plural();
    test_time_table_is_aligned();
}

} // namespace spike_tests
