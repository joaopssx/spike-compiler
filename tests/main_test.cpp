// Lightweight test runner for the Spike Compiler.
// Each test increments a failure counter on assertion error; the runner ends
// with "all tests passed" iff zero failures occurred, which is what CTest
// matches against.

#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/lexer.hpp"
#include "spike/parser.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <variant>

namespace {

// ---- parser (PROMPT 014) --------------------------------------------------

// Helper: lex + parse `src` e retorna o Program junto com o parser pra
// inspecionar diagnósticos.
struct ParseResult {
    spike::Program program;
    bool had_error;
    std::string diag_output;
};

ParseResult parse_source(const std::string& src) {
    spike::Lexer lex(src, "<test>");
    auto tokens = lex.tokenize();
    spike::Parser parser(std::move(tokens), "<test>", src);
    ParseResult r;
    r.program = parser.parse();
    r.had_error = parser.had_error();
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    parser.diagnostics().print_all();
    std::cerr.rdbuf(old);
    r.diag_output = sink.str();
    return r;
}

void test_parser_empty_program() {
    auto r = parse_source("algoritmo \"vazio\"\ninicio\nfimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.name == "vazio");
    SPIKE_EXPECT(r.program.globals.empty());
    SPIKE_EXPECT(r.program.body.empty());
}

void test_parser_single_var_decl() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "var\n"
        "   x : inteiro\n"
        "inicio\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.globals.size() == 1);
    SPIKE_EXPECT(r.program.globals[0].name == "x");
    SPIKE_EXPECT(r.program.globals[0].type == "inteiro");
    SPIKE_EXPECT(r.program.globals[0].initializer == nullptr);
}

void test_parser_multi_name_var_decl() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "var\n"
        "   a, b, c : real\n"
        "inicio\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.globals.size() == 3);
    SPIKE_EXPECT(r.program.globals[0].name == "a");
    SPIKE_EXPECT(r.program.globals[1].name == "b");
    SPIKE_EXPECT(r.program.globals[2].name == "c");
    for (const auto& g : r.program.globals) {
        SPIKE_EXPECT(g.type == "real");
    }
}

void test_parser_multiple_var_lines() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "var\n"
        "   x : inteiro\n"
        "   nome : caractere\n"
        "   ok : logico\n"
        "inicio\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.globals.size() == 3);
    SPIKE_EXPECT(r.program.globals[0].type == "inteiro");
    SPIKE_EXPECT(r.program.globals[1].type == "caractere");
    SPIKE_EXPECT(r.program.globals[2].type == "logico");
}

void test_parser_E100_missing_algoritmo() {
    auto r = parse_source("inicio fimalgoritmo");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E100") != std::string::npos);
}

void test_parser_E101_missing_name() {
    auto r = parse_source("algoritmo inicio fimalgoritmo");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E101") != std::string::npos);
}

void test_parser_E102_missing_inicio() {
    auto r = parse_source("algoritmo \"t\"\nfimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E102") != std::string::npos);
}

void test_parser_E103_missing_fimalgoritmo() {
    auto r = parse_source("algoritmo \"t\"\ninicio\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E103") != std::string::npos);
}

void test_parser_E104_bad_type() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "var\n"
        "   x : numero\n"
        "inicio\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E104") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find("numero") != std::string::npos);
}

void test_parser_E105_missing_colon() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "var\n"
        "   x inteiro\n"
        "inicio\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E105") != std::string::npos);
}

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

// ---- parser: se statement (PROMPT 017) ------------------------------------

void test_stmt_se_then_only() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   se x > 0 entao\n"
        "      escreva(\"pos\")\n"
        "   fimse\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.body.size() == 1);
    SPIKE_EXPECT(std::holds_alternative<spike::SeStmt>(r.program.body[0]->node));
    const auto& s = std::get<spike::SeStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(std::holds_alternative<spike::BinaryExpr>(s.condition->node));
    SPIKE_EXPECT(std::get<spike::BinaryExpr>(s.condition->node).op == ">");
    SPIKE_EXPECT(s.then_branch.size() == 1);
    SPIKE_EXPECT(s.else_branch.empty());
    SPIKE_EXPECT(std::holds_alternative<spike::EscrevaStmt>(s.then_branch[0]->node));
}

void test_stmt_se_with_senao() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   se x > 0 entao\n"
        "      escreva(\"pos\")\n"
        "   senao\n"
        "      escreva(\"neg\")\n"
        "   fimse\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    const auto& s = std::get<spike::SeStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(s.then_branch.size() == 1);
    SPIKE_EXPECT(s.else_branch.size() == 1);
    SPIKE_EXPECT(std::holds_alternative<spike::EscrevaStmt>(s.then_branch[0]->node));
    SPIKE_EXPECT(std::holds_alternative<spike::EscrevaStmt>(s.else_branch[0]->node));
}

void test_stmt_se_nested() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   se a entao\n"
        "      se b entao\n"
        "         escreva(\"nested\")\n"
        "      fimse\n"
        "   fimse\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    const auto& outer = std::get<spike::SeStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(outer.then_branch.size() == 1);
    SPIKE_EXPECT(std::holds_alternative<spike::SeStmt>(outer.then_branch[0]->node));
    const auto& inner = std::get<spike::SeStmt>(outer.then_branch[0]->node);
    SPIKE_EXPECT(inner.then_branch.size() == 1);
    SPIKE_EXPECT(std::holds_alternative<spike::EscrevaStmt>(inner.then_branch[0]->node));
}

void test_stmt_E400_missing_entao() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   se x > 0\n"
        "      escreva(\"oi\")\n"
        "   fimse\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E400") != std::string::npos);
}

// ---- parser: enquanto/para (PROMPT 018) -----------------------------------

void test_stmt_enquanto_basic() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   enquanto i < 10 faca\n"
        "      i <- i + 1\n"
        "   fimenquanto\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.body.size() == 1);
    SPIKE_EXPECT(std::holds_alternative<spike::EnquantoStmt>(r.program.body[0]->node));
    const auto& w = std::get<spike::EnquantoStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(std::holds_alternative<spike::BinaryExpr>(w.condition->node));
    SPIKE_EXPECT(std::get<spike::BinaryExpr>(w.condition->node).op == "<");
    SPIKE_EXPECT(w.body.size() == 1);
    SPIKE_EXPECT(std::holds_alternative<spike::AssignStmt>(w.body[0]->node));
}

void test_stmt_para_no_step() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   para i de 1 ate 5 faca\n"
        "      escreva(i)\n"
        "   fimpara\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(std::holds_alternative<spike::ParaStmt>(r.program.body[0]->node));
    const auto& p = std::get<spike::ParaStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(p.var_name == "i");
    SPIKE_EXPECT(std::get<spike::NumberExpr>(p.from->node).value == 1.0);
    SPIKE_EXPECT(std::get<spike::NumberExpr>(p.to->node).value == 5.0);
    SPIKE_EXPECT(p.step == nullptr);
    SPIKE_EXPECT(p.body.size() == 1);
}

void test_stmt_para_with_step() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   para i de 10 ate 1 passo -1 faca\n"
        "      escreva(i)\n"
        "   fimpara\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    const auto& p = std::get<spike::ParaStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(p.var_name == "i");
    SPIKE_EXPECT(std::get<spike::NumberExpr>(p.from->node).value == 10.0);
    SPIKE_EXPECT(std::get<spike::NumberExpr>(p.to->node).value == 1.0);
    SPIKE_EXPECT(p.step != nullptr);
    // step é UnaryExpr("-", NumberExpr(1))
    SPIKE_EXPECT(std::holds_alternative<spike::UnaryExpr>(p.step->node));
    const auto& neg = std::get<spike::UnaryExpr>(p.step->node);
    SPIKE_EXPECT(neg.op == "-");
    SPIKE_EXPECT(std::get<spike::NumberExpr>(neg.operand->node).value == 1.0);
}

void test_stmt_enquanto_nested_in_para() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   para i de 1 ate 3 faca\n"
        "      enquanto j > 0 faca\n"
        "         j <- j - 1\n"
        "      fimenquanto\n"
        "   fimpara\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    const auto& p = std::get<spike::ParaStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(p.body.size() == 1);
    SPIKE_EXPECT(std::holds_alternative<spike::EnquantoStmt>(p.body[0]->node));
}

void test_stmt_E500_enquanto_missing_faca() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   enquanto i < 10\n"
        "      i <- i + 1\n"
        "   fimenquanto\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E500") != std::string::npos);
}

void test_stmt_E501_enquanto_missing_fimenquanto() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   enquanto i < 10 faca\n"
        "      i <- i + 1\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E501") != std::string::npos);
}

void test_stmt_E503_para_missing_de() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   para i 1 ate 5 faca\n"
        "      escreva(i)\n"
        "   fimpara\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E503") != std::string::npos);
}

void test_stmt_E504_para_missing_ate() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   para i de 1 5 faca\n"
        "      escreva(i)\n"
        "   fimpara\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E504") != std::string::npos);
}

// ---- parser: repita statement (PROMPT 019) --------------------------------

void test_stmt_repita_basic() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   repita\n"
        "      leia(x)\n"
        "   ate x > 0\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.program.body.size() == 1);
    SPIKE_EXPECT(std::holds_alternative<spike::RepitaStmt>(r.program.body[0]->node));
    const auto& rp = std::get<spike::RepitaStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(rp.body.size() == 1);
    SPIKE_EXPECT(std::holds_alternative<spike::LeiaStmt>(rp.body[0]->node));
    SPIKE_EXPECT(std::holds_alternative<spike::BinaryExpr>(rp.condition->node));
    SPIKE_EXPECT(std::get<spike::BinaryExpr>(rp.condition->node).op == ">");
}

void test_stmt_repita_empty_body() {
    // Body vazio é sintaticamente válido (semanticamente esquisito).
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   repita\n"
        "   ate verdadeiro\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    const auto& rp = std::get<spike::RepitaStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(rp.body.empty());
    SPIKE_EXPECT(std::holds_alternative<spike::BoolExpr>(rp.condition->node));
    SPIKE_EXPECT(std::get<spike::BoolExpr>(rp.condition->node).value == true);
}

void test_stmt_repita_nested_in_enquanto() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   enquanto x > 0 faca\n"
        "      repita\n"
        "         leia(y)\n"
        "      ate y = 0\n"
        "      x <- x - 1\n"
        "   fimenquanto\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == false);
    const auto& w = std::get<spike::EnquantoStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(w.body.size() == 2);
    SPIKE_EXPECT(std::holds_alternative<spike::RepitaStmt>(w.body[0]->node));
    SPIKE_EXPECT(std::holds_alternative<spike::AssignStmt>(w.body[1]->node));
}

void test_stmt_E600_repita_missing_ate() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   repita\n"
        "      leia(x)\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E600") != std::string::npos);
}

void test_stmt_E506_para_missing_fimpara() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   para i de 1 ate 5 faca\n"
        "      escreva(i)\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E506") != std::string::npos);
}

void test_stmt_E401_missing_fimse() {
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "inicio\n"
        "   se x entao\n"
        "      escreva(\"oi\")\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    SPIKE_EXPECT(r.diag_output.find("E401") != std::string::npos);
}

// ---- parser: expressions (PROMPT 015) -------------------------------------

// Helper: lex `src` e parseia uma única expressão.
struct ExprResult {
    spike::ExprPtr expr;
    bool had_error;
};

ExprResult parse_expression(const std::string& src) {
    spike::Lexer lex(src, "<test>");
    auto tokens = lex.tokenize();
    spike::Parser parser(std::move(tokens), "<test>", src);
    ExprResult r;
    r.expr = parser.parse_expr();
    r.had_error = parser.had_error();
    return r;
}

void test_expr_number_literal() {
    auto r = parse_expression("42");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(r.expr != nullptr);
    SPIKE_EXPECT(std::holds_alternative<spike::NumberExpr>(r.expr->node));
    const auto& n = std::get<spike::NumberExpr>(r.expr->node);
    SPIKE_EXPECT(n.value == 42.0);
    SPIKE_EXPECT(n.is_integer == true);
}

void test_expr_real_literal() {
    auto r = parse_expression("3.14");
    SPIKE_EXPECT(r.had_error == false);
    const auto& n = std::get<spike::NumberExpr>(r.expr->node);
    SPIKE_EXPECT(n.value > 3.13 && n.value < 3.15);
    SPIKE_EXPECT(n.is_integer == false);
}

void test_expr_var() {
    auto r = parse_expression("contador");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(std::holds_alternative<spike::VarExpr>(r.expr->node));
    SPIKE_EXPECT(std::get<spike::VarExpr>(r.expr->node).name == "contador");
}

void test_expr_bool_literals() {
    {
        auto r = parse_expression("verdadeiro");
        SPIKE_EXPECT(r.had_error == false);
        SPIKE_EXPECT(std::get<spike::BoolExpr>(r.expr->node).value == true);
    }
    {
        auto r = parse_expression("falso");
        SPIKE_EXPECT(r.had_error == false);
        SPIKE_EXPECT(std::get<spike::BoolExpr>(r.expr->node).value == false);
    }
}

void test_expr_addition() {
    auto r = parse_expression("x + 1");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(std::holds_alternative<spike::BinaryExpr>(r.expr->node));
    const auto& b = std::get<spike::BinaryExpr>(r.expr->node);
    SPIKE_EXPECT(b.op == "+");
    SPIKE_EXPECT(std::holds_alternative<spike::VarExpr>(b.left->node));
    SPIKE_EXPECT(std::get<spike::VarExpr>(b.left->node).name == "x");
    SPIKE_EXPECT(std::holds_alternative<spike::NumberExpr>(b.right->node));
    SPIKE_EXPECT(std::get<spike::NumberExpr>(b.right->node).value == 1.0);
}

void test_expr_unary_not() {
    auto r = parse_expression("nao verdadeiro");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(std::holds_alternative<spike::UnaryExpr>(r.expr->node));
    const auto& u = std::get<spike::UnaryExpr>(r.expr->node);
    SPIKE_EXPECT(u.op == "nao");
    SPIKE_EXPECT(std::holds_alternative<spike::BoolExpr>(u.operand->node));
    SPIKE_EXPECT(std::get<spike::BoolExpr>(u.operand->node).value == true);
}

void test_expr_paren_changes_precedence() {
    // (3 + 4) * 2 → raiz é o '*', filho esquerdo é o '+'.
    auto r = parse_expression("(3 + 4) * 2");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(std::holds_alternative<spike::BinaryExpr>(r.expr->node));
    const auto& mul = std::get<spike::BinaryExpr>(r.expr->node);
    SPIKE_EXPECT(mul.op == "*");
    SPIKE_EXPECT(std::holds_alternative<spike::BinaryExpr>(mul.left->node));
    SPIKE_EXPECT(std::get<spike::BinaryExpr>(mul.left->node).op == "+");
    SPIKE_EXPECT(std::holds_alternative<spike::NumberExpr>(mul.right->node));
    SPIKE_EXPECT(std::get<spike::NumberExpr>(mul.right->node).value == 2.0);
}

void test_expr_precedence_without_parens() {
    // 1 + 2 * 3 → 1 + (2 * 3); raiz é '+'.
    auto r = parse_expression("1 + 2 * 3");
    SPIKE_EXPECT(r.had_error == false);
    const auto& add = std::get<spike::BinaryExpr>(r.expr->node);
    SPIKE_EXPECT(add.op == "+");
    SPIKE_EXPECT(std::holds_alternative<spike::NumberExpr>(add.left->node));
    SPIKE_EXPECT(std::get<spike::NumberExpr>(add.left->node).value == 1.0);
    const auto& mul = std::get<spike::BinaryExpr>(add.right->node);
    SPIKE_EXPECT(mul.op == "*");
}

void test_expr_comparison_lower_than_arithmetic() {
    // a + 1 < b * 2 → comparação na raiz.
    auto r = parse_expression("a + 1 < b * 2");
    SPIKE_EXPECT(r.had_error == false);
    const auto& cmp = std::get<spike::BinaryExpr>(r.expr->node);
    SPIKE_EXPECT(cmp.op == "<");
}

void test_expr_and_lower_than_comparison() {
    // a > 1 e b < 2 → 'e' na raiz, comparações nos filhos.
    auto r = parse_expression("a > 1 e b < 2");
    SPIKE_EXPECT(r.had_error == false);
    const auto& and_node = std::get<spike::BinaryExpr>(r.expr->node);
    SPIKE_EXPECT(and_node.op == "e");
    SPIKE_EXPECT(std::get<spike::BinaryExpr>(and_node.left->node).op == ">");
    SPIKE_EXPECT(std::get<spike::BinaryExpr>(and_node.right->node).op == "<");
}

void test_expr_or_keyword_case_insensitive() {
    // Lexer normaliza keywords no match; canonical_op força lowercase no op.
    auto r = parse_expression("verdadeiro OU falso");
    SPIKE_EXPECT(r.had_error == false);
    SPIKE_EXPECT(std::get<spike::BinaryExpr>(r.expr->node).op == "ou");
}

void test_expr_unary_minus() {
    auto r = parse_expression("-x");
    SPIKE_EXPECT(r.had_error == false);
    const auto& u = std::get<spike::UnaryExpr>(r.expr->node);
    SPIKE_EXPECT(u.op == "-");
    SPIKE_EXPECT(std::get<spike::VarExpr>(u.operand->node).name == "x");
}

void test_expr_unclosed_paren_emits_E200() {
    spike::Lexer lex("(1 + 2", "<test>");
    auto toks = lex.tokenize();
    spike::Parser p(std::move(toks), "<test>", "(1 + 2");
    p.parse_expr();
    SPIKE_EXPECT(p.had_error());
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    p.diagnostics().print_all();
    std::cerr.rdbuf(old);
    SPIKE_EXPECT(sink.str().find("E200") != std::string::npos);
}

void test_expr_unexpected_token_emits_E201() {
    spike::Lexer lex("+", "<test>");
    auto toks = lex.tokenize();
    spike::Parser p(std::move(toks), "<test>", "+");
    p.parse_expr();
    SPIKE_EXPECT(p.had_error());
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    p.diagnostics().print_all();
    std::cerr.rdbuf(old);
    SPIKE_EXPECT(sink.str().find("E201") != std::string::npos);
}

void test_parser_recovers_to_next_line_after_var_error() {
    // Linha ruim, depois uma linha boa — a boa deve ser parseada.
    auto r = parse_source(
        "algoritmo \"t\"\n"
        "var\n"
        "   x foobar lixo\n"            // erro: sem ':'
        "   y : inteiro\n"
        "inicio\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(r.had_error == true);
    // y deveria ter sido capturado mesmo depois do erro em x.
    bool found_y = false;
    for (const auto& g : r.program.globals) {
        if (g.name == "y" && g.type == "inteiro") found_y = true;
    }
    SPIKE_EXPECT(found_y);
}

} // namespace

int main() {
    spike_tests::run_file_reader_tests();
    spike_tests::run_token_tests();
    spike_tests::run_lexer_keyword_tests();
    spike_tests::run_lexer_number_tests();
    spike_tests::run_lexer_string_tests();
    spike_tests::run_lexer_operator_tests();
    spike_tests::run_lexer_comment_tests();
    spike_tests::run_lexer_error_tests();
    spike_tests::run_diagnostics_tests();
    spike_tests::run_cli_tests();
    spike_tests::run_ast_tests();

    test_parser_empty_program();
    test_parser_single_var_decl();
    test_parser_multi_name_var_decl();
    test_parser_multiple_var_lines();
    test_parser_E100_missing_algoritmo();
    test_parser_E101_missing_name();
    test_parser_E102_missing_inicio();
    test_parser_E103_missing_fimalgoritmo();
    test_parser_E104_bad_type();
    test_parser_E105_missing_colon();
    test_parser_recovers_to_next_line_after_var_error();
    test_expr_number_literal();
    test_expr_real_literal();
    test_expr_var();
    test_expr_bool_literals();
    test_expr_addition();
    test_expr_unary_not();
    test_expr_paren_changes_precedence();
    test_expr_precedence_without_parens();
    test_expr_comparison_lower_than_arithmetic();
    test_expr_and_lower_than_comparison();
    test_expr_or_keyword_case_insensitive();
    test_expr_unary_minus();
    test_expr_unclosed_paren_emits_E200();
    test_expr_unexpected_token_emits_E201();
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
    test_stmt_se_then_only();
    test_stmt_se_with_senao();
    test_stmt_se_nested();
    test_stmt_E400_missing_entao();
    test_stmt_E401_missing_fimse();
    test_stmt_enquanto_basic();
    test_stmt_para_no_step();
    test_stmt_para_with_step();
    test_stmt_enquanto_nested_in_para();
    test_stmt_E500_enquanto_missing_faca();
    test_stmt_E501_enquanto_missing_fimenquanto();
    test_stmt_E503_para_missing_de();
    test_stmt_E504_para_missing_ate();
    test_stmt_E506_para_missing_fimpara();
    test_stmt_repita_basic();
    test_stmt_repita_empty_body();
    test_stmt_repita_nested_in_enquanto();
    test_stmt_E600_repita_missing_ate();

    if (spike_tests::g_failures == 0) {
        std::cout << "all tests passed" << std::endl;
        return 0;
    }
    std::cerr << spike_tests::g_failures << " test(s) failed" << std::endl;
    return 1;
}
