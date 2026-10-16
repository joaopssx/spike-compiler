#include "parser/parser_support.hpp"
#include "semantic/semantic_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/ast.hpp"
#include "spike/ast_printer.hpp"
#include "spike/token.hpp"

#include <string>
#include <variant>

namespace {

using spike_tests::analyze_typed;
using spike_tests::count_occurrences;
using spike_tests::parse_source;
using spike_tests::program_with;
using spike_tests::tokens_of;

std::string errors_of(const std::string& body) {
    return analyze_typed(program_with("  i : inteiro\n  b : logico\n", body))->diag_output;
}

void test_lexer_and_parser_build_break() {
    const auto toks = tokens_of("interrompa INTERROMPA interrompido");
    SPIKE_EXPECT(toks.size() == 3 && toks[0].type == spike::TokenType::INTERROMPA &&
                 toks[1].type == spike::TokenType::INTERROMPA &&
                 toks[2].type == spike::TokenType::IDENTIFIER);

    auto r = parse_source("algoritmo \"t\"\ninicio\n  enquanto verdadeiro faca\n    interrompa\n"
                          "  fimenquanto\nfimalgoritmo\n");
    SPIKE_EXPECT(!r.had_error);
    const auto& loop = std::get<spike::EnquantoStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(loop.body.size() == 1);
    const auto* brk = std::get_if<spike::BreakStmt>(&loop.body[0]->node);
    SPIKE_EXPECT(brk && brk->line == 4 && brk->col == 5);
    SPIKE_EXPECT(spike::print_ast(r.program).find("    faca\n      Interrompa (4:5)\n") !=
                 std::string::npos);
}

void test_break_inside_each_loop_is_valid() {
    SPIKE_EXPECT(errors_of("  enquanto b faca\n    interrompa\n  fimenquanto\n").empty());
    SPIKE_EXPECT(errors_of("  repita\n    interrompa\n  ate b\n").empty());
    SPIKE_EXPECT(errors_of("  para i de 1 ate 3 faca\n    interrompa\n  fimpara\n").empty());
}

void test_break_inside_se_inside_loop_is_valid() {
    SPIKE_EXPECT(errors_of("  para i de 1 ate 3 faca\n    se i = 2 entao\n      interrompa\n"
                           "    senao\n      interrompa\n    fimse\n  fimpara\n")
                     .empty());
}

void test_break_outside_loop_is_e750() {
    const std::string out = errors_of("  interrompa\n");
    SPIKE_EXPECT(count_occurrences(out, "erro[") == 1);
    SPIKE_EXPECT(out.find("[E750]") != std::string::npos);
    SPIKE_EXPECT(out.find("'interrompa' só pode ser usado dentro de um laço") != std::string::npos);
    SPIKE_EXPECT(out.find("enquanto, repita ou para") != std::string::npos);
    SPIKE_EXPECT(out.find(":6:3") != std::string::npos);
}

void test_break_in_se_outside_loop_is_e750() {
    SPIKE_EXPECT(errors_of("  se b entao\n    interrompa\n  fimse\n").find("[E750]") !=
                 std::string::npos);
}

void test_break_after_loop_is_e750() {
    const std::string out = errors_of("  enquanto b faca\n  fimenquanto\n  interrompa\n");
    SPIKE_EXPECT(count_occurrences(out, "[E750]") == 1);
}

} // namespace

namespace spike_tests {

void run_semantic_break_tests() {
    test_lexer_and_parser_build_break();
    test_break_inside_each_loop_is_valid();
    test_break_inside_se_inside_loop_is_valid();
    test_break_outside_loop_is_e750();
    test_break_in_se_outside_loop_is_e750();
    test_break_after_loop_is_e750();
}

} // namespace spike_tests
