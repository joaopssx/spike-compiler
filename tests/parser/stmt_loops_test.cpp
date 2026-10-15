#include "parser/parser_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/ast.hpp"

#include <string>
#include <variant>

namespace {

using spike_tests::parse_source;

// ---- parser: enquanto/para ------------------------------------------------
// ---- parser: repita statement ---------------------------------------------

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
    SPIKE_EXPECT(std::get<spike::BinaryExpr>(w.condition->node).op == spike::BinaryOp::Lt);
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
    SPIKE_EXPECT(neg.op == spike::UnaryOp::Neg);
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
    SPIKE_EXPECT(std::get<spike::BinaryExpr>(rp.condition->node).op == spike::BinaryOp::Gt);
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

} // namespace

namespace spike_tests {

void run_parser_stmt_loop_tests() {
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
}

} // namespace spike_tests
