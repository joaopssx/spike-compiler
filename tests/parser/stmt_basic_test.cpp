#include "parser/parser_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/ast.hpp"

#include <string>
#include <variant>

namespace {

using spike_tests::parse_source;

// ---- parser: statements (PROMPT 016) --------------------------------------

void test_stmt_assignment_arrow() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   x <- 5\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.body.size() == 1);
    SPIKE_EXPECT(std::holds_alternative<spike::AssignStmt>(r.program.body[0]->node));
    const auto& a = std::get<spike::AssignStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(a.var_name == "x");
    SPIKE_EXPECT(std::holds_alternative<spike::NumberExpr>(a.value->node));
    SPIKE_EXPECT(std::get<spike::NumberExpr>(a.value->node).value == 5.0);
}

void test_stmt_assignment_pascal_style() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   x := 7\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.body.size() == 1);
    const auto& a = std::get<spike::AssignStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(a.var_name == "x");
    SPIKE_EXPECT(std::get<spike::NumberExpr>(a.value->node).value == 7.0);
}

void test_stmt_escreva_two_args() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   escreva(\"oi\", x)\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.body.size() == 1);
    SPIKE_EXPECT(std::holds_alternative<spike::EscrevaStmt>(r.program.body[0]->node));
    const auto& e = std::get<spike::EscrevaStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(e.newline == false);
    SPIKE_EXPECT(e.args.size() == 2);
    SPIKE_EXPECT(std::holds_alternative<spike::StringExpr>(e.args[0]->node));
    SPIKE_EXPECT(std::get<spike::StringExpr>(e.args[0]->node).value == "oi");
    SPIKE_EXPECT(std::holds_alternative<spike::VarExpr>(e.args[1]->node));
    SPIKE_EXPECT(std::get<spike::VarExpr>(e.args[1]->node).name == "x");
}

void test_stmt_escreval_sets_newline() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   escreval(\"linha\")\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    const auto& e = std::get<spike::EscrevaStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(e.newline == true);
    SPIKE_EXPECT(e.args.size() == 1);
}

void test_stmt_escreva_empty_args() {
    // escreval() é útil pra imprimir linha em branco — não deve dar erro.
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   escreval()\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    const auto& e = std::get<spike::EscrevaStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(e.args.empty());
    SPIKE_EXPECT(e.newline == true);
}

void test_stmt_leia_two_names() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   leia(x, y)\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.body.size() == 1);
    SPIKE_EXPECT(std::holds_alternative<spike::LeiaStmt>(r.program.body[0]->node));
    const auto& l = std::get<spike::LeiaStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(l.var_names.size() == 2);
    SPIKE_EXPECT(l.var_names[0] == "x");
    SPIKE_EXPECT(l.var_names[1] == "y");
}

void test_stmt_multiple_statements_in_body() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "var x : inteiro\n"
        "inicio\n"
        "   x <- 1\n"
        "   escreva(x)\n"
        "   leia(x)\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.body.size() == 3);
    SPIKE_EXPECT(std::holds_alternative<spike::AssignStmt>(r.program.body[0]->node));
    SPIKE_EXPECT(std::holds_alternative<spike::EscrevaStmt>(r.program.body[1]->node));
    SPIKE_EXPECT(std::holds_alternative<spike::LeiaStmt>(r.program.body[2]->node));
}

void test_stmt_E300_unknown_statement() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   42\n"           // 42 sozinho não é statement válido
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E300") != std::string::npos);
}

void test_stmt_E301_escreva_missing_lparen() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   escreva \"oi\")\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E301") != std::string::npos);
}

void test_stmt_E302_escreva_missing_rparen() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   escreva(\"oi\"\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E302") != std::string::npos);
}

void test_stmt_E303_leia_missing_lparen() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   leia x)\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E303") != std::string::npos);
}

void test_stmt_E304_leia_missing_rparen() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   leia(x\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E304") != std::string::npos);
}

} // namespace

namespace spike_tests {

void run_parser_stmt_basic_tests() {
    test_stmt_assignment_arrow();
    test_stmt_assignment_pascal_style();
    test_stmt_escreva_two_args();
    test_stmt_escreval_sets_newline();
    test_stmt_escreva_empty_args();
    test_stmt_leia_two_names();
    test_stmt_multiple_statements_in_body();
    test_stmt_E300_unknown_statement();
    test_stmt_E301_escreva_missing_lparen();
    test_stmt_E302_escreva_missing_rparen();
    test_stmt_E303_leia_missing_lparen();
    test_stmt_E304_leia_missing_rparen();
}

} // namespace spike_tests
