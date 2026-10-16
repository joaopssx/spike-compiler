#include "parser/parser_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/ast.hpp"
#include "spike/ast_printer.hpp"
#include "spike/token.hpp"

#include <string>
#include <variant>

namespace {

using spike_tests::parse_source;
using spike_tests::tokens_of;

std::string with_consts(const std::string& consts) {
    return "algoritmo \"t\"\nconst\n" + consts + "var\n  x : inteiro\ninicio\nfimalgoritmo\n";
}

void test_lexer_const_keyword() {
    const auto toks = tokens_of("const CONST constante");
    SPIKE_EXPECT(toks.size() == 3);
    if (toks.size() != 3) return;
    SPIKE_EXPECT(toks[0].type == spike::TokenType::CONST_KEYWORD);
    SPIKE_EXPECT(toks[1].type == spike::TokenType::CONST_KEYWORD);
    SPIKE_EXPECT(toks[2].type == spike::TokenType::IDENTIFIER);
}

void test_const_section_is_parsed() {
    auto r = parse_source(with_consts("  PI = 3.14159\n  LIMITE = 10\n  NOME = \"Area\"\n"
                                      "  DOBRO = LIMITE * 2\n"));
    SPIKE_EXPECT(!r.had_error);
    SPIKE_EXPECT(r.program.consts.size() == 4);
    if (r.program.consts.size() != 4) return;
    SPIKE_EXPECT(r.program.consts[0].name == "PI");
    SPIKE_EXPECT(r.program.consts[0].line == 3);
    SPIKE_EXPECT(r.program.consts[0].col == 3);
    SPIKE_EXPECT(std::holds_alternative<spike::BinaryExpr>(r.program.consts[3].value->node));
    SPIKE_EXPECT(r.program.globals.size() == 1);
}

void test_const_section_before_inicio_without_var() {
    auto r = parse_source("algoritmo \"t\"\nconst\n  N = 1\ninicio\nfimalgoritmo\n");
    SPIKE_EXPECT(!r.had_error);
    SPIKE_EXPECT(r.program.consts.size() == 1);
    SPIKE_EXPECT(r.program.globals.empty());
}

void test_program_without_const_is_valid() {
    auto r = parse_source("algoritmo \"t\"\nvar\n  x : inteiro\ninicio\nfimalgoritmo\n");
    SPIKE_EXPECT(!r.had_error);
    SPIKE_EXPECT(r.program.consts.empty());
}

void test_empty_const_section_is_e800() {
    auto r = parse_source("algoritmo \"t\"\nconst\nvar\n  x : inteiro\ninicio\nfimalgoritmo\n");
    SPIKE_EXPECT(r.diag_output.find("[E800]") != std::string::npos);
}

void test_const_without_name_is_e800() {
    auto r = parse_source(with_consts("  = 3\n  OK = 1\n"));
    SPIKE_EXPECT(r.diag_output.find("[E800]") != std::string::npos);
    SPIKE_EXPECT(r.program.consts.size() == 1);
}

void test_const_without_equal_is_e801() {
    auto r = parse_source(with_consts("  PI 3.14\n  OK = 1\n"));
    SPIKE_EXPECT(r.diag_output.find("[E801]") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find("'PI'") != std::string::npos);
    SPIKE_EXPECT(r.program.consts.size() == 1);
}

void test_const_without_value_is_e802() {
    auto r = parse_source(with_consts("  PI =\n  OK = 1\n"));
    SPIKE_EXPECT(r.diag_output.find("[E802]") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find(":3:7") != std::string::npos);
    SPIKE_EXPECT(r.program.consts.size() == 1);
}

void test_ast_printer_shows_constants() {
    auto r = parse_source("algoritmo \"t\"\nconst\n  N = 2 + 1\ninicio\nfimalgoritmo\n");
    SPIKE_EXPECT(spike::print_ast(r.program) ==
                 "Program \"t\" (1:1)\n"
                 "  Const N (3:3)\n"
                 "    Binary + (3:9)\n"
                 "      Number 2\n"
                 "      Number 1\n");
}

} // namespace

namespace spike_tests {

void run_parser_const_tests() {
    test_lexer_const_keyword();
    test_const_section_is_parsed();
    test_const_section_before_inicio_without_var();
    test_program_without_const_is_valid();
    test_empty_const_section_is_e800();
    test_const_without_name_is_e800();
    test_const_without_equal_is_e801();
    test_const_without_value_is_e802();
    test_ast_printer_shows_constants();
}

} // namespace spike_tests
