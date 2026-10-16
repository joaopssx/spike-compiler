#include "parser/parser_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/ast.hpp"
#include "spike/ast_printer.hpp"
#include "spike/token.hpp"

#include <string>
#include <variant>
#include <vector>

namespace {

using spike::TokenType;
using spike_tests::parse_source;
using spike_tests::tokens_of;

std::vector<TokenType> types_of(const std::string& src) {
    std::vector<TokenType> types;
    for (const auto& t : tokens_of(src)) types.push_back(t.type);
    return types;
}

std::string with_var(const std::string& decl) {
    return "algoritmo \"t\"\nconst\n  N = 3\nvar\n" + decl + "inicio\nfimalgoritmo\n";
}

void test_lexer_range_dots() {
    SPIKE_EXPECT((types_of("1..10") ==
                  std::vector<TokenType>{TokenType::NUMBER, TokenType::DOTDOT, TokenType::NUMBER}));
    SPIKE_EXPECT((types_of("1 ..10") ==
                  std::vector<TokenType>{TokenType::NUMBER, TokenType::DOTDOT, TokenType::NUMBER}));
    SPIKE_EXPECT((types_of("a[1..2]") ==
                  std::vector<TokenType>{TokenType::IDENTIFIER, TokenType::LBRACKET,
                                         TokenType::NUMBER, TokenType::DOTDOT, TokenType::NUMBER,
                                         TokenType::RBRACKET}));
    const auto real = tokens_of("1.5");
    SPIKE_EXPECT(real.size() == 1 && real[0].lexeme == "1.5");
    spike::Lexer lex("1.", "<test>");
    lex.tokenize();
    SPIKE_EXPECT(lex.had_error());
    SPIKE_EXPECT((types_of("vetor VETOR vetores") ==
                  std::vector<TokenType>{TokenType::VETOR, TokenType::VETOR, TokenType::IDENTIFIER}));
}

void test_array_declaration() {
    auto r = parse_source(with_var("  notas, extra : vetor[1..5] de real\n  z : vetor[-2..N] de inteiro\n"
                                   "  x : inteiro\n"));
    SPIKE_EXPECT(!r.had_error);
    SPIKE_EXPECT(r.program.globals.size() == 4);
    if (r.program.globals.size() != 4) return;
    const auto& notas = r.program.globals[0];
    SPIKE_EXPECT(notas.type == spike::TypeKind::Real);
    SPIKE_EXPECT(notas.dims.size() == 1 && notas.dims[0].low.value == 1 &&
                 notas.dims[0].high.value == 5);
    SPIKE_EXPECT(r.program.globals[1].dims.size() == 1);
    const auto& z = r.program.globals[2];
    SPIKE_EXPECT(z.dims[0].low.value == -2 && z.dims[0].high.constant == "N");
    SPIKE_EXPECT(r.program.globals[3].dims.empty());
    SPIKE_EXPECT(spike::print_ast(r.program).find("Var notas : vetor[1..5] de real (5:3)") !=
                 std::string::npos);
}

void test_matrix_declaration() {
    auto r = parse_source(with_var("  m : vetor[1..3, 1..4] de inteiro\n"
                                   "  c : vetor[0..1, -1..1, 1..N] de real\n"));
    SPIKE_EXPECT(!r.had_error);
    SPIKE_EXPECT(r.program.globals.size() == 2);
    if (r.program.globals.size() != 2) return;
    const auto& m = r.program.globals[0];
    SPIKE_EXPECT(m.dims.size() == 2 && m.dims[1].low.value == 1 && m.dims[1].high.value == 4);
    const auto& c = r.program.globals[1];
    SPIKE_EXPECT(c.dims.size() == 3 && c.dims[1].low.value == -1 && c.dims[2].high.constant == "N");
    SPIKE_EXPECT(spike::print_ast(r.program).find("Var m : vetor[1..3, 1..4] de inteiro (5:3)") !=
                 std::string::npos);
}

void test_matrix_use() {
    auto r = parse_source("algoritmo \"t\"\nvar\n  m : vetor[1..2, 1..2] de inteiro\n  i : inteiro\n"
                          "inicio\n  m[i, 2] <- m[1, i + 1]\nfimalgoritmo\n");
    SPIKE_EXPECT(!r.had_error);
    const auto& assign = std::get<spike::AssignStmt>(r.program.body[0]->node);
    const auto* target = std::get_if<spike::IndexExpr>(&assign.target->node);
    SPIKE_EXPECT(target && target->indices.size() == 2);
    const auto* value = std::get_if<spike::IndexExpr>(&assign.value->node);
    SPIKE_EXPECT(value && value->indices.size() == 2 &&
                 std::holds_alternative<spike::BinaryExpr>(value->indices[1]->node));
}

void test_array_use_in_expressions_and_targets() {
    auto r = parse_source("algoritmo \"t\"\nvar\n  v : vetor[1..3] de inteiro\n  i : inteiro\ninicio\n"
                          "  v[i + 1] <- v[1] * 2\n  leia(v[2], i)\nfimalgoritmo\n");
    SPIKE_EXPECT(!r.had_error);
    const auto& assign = std::get<spike::AssignStmt>(r.program.body[0]->node);
    const auto* target = std::get_if<spike::IndexExpr>(&assign.target->node);
    SPIKE_EXPECT(target && target->name == "v" && target->indices.size() == 1);
    SPIKE_EXPECT(target && target->line == 6 && target->col == 3);
    const auto& value = std::get<spike::BinaryExpr>(assign.value->node);
    SPIKE_EXPECT(std::holds_alternative<spike::IndexExpr>(value.left->node));
    const auto& leia = std::get<spike::LeiaStmt>(r.program.body[1]->node);
    SPIKE_EXPECT(leia.targets.size() == 2 &&
                 std::holds_alternative<spike::IndexExpr>(leia.targets[0]->node));
}

void expect_error(const std::string& decl, const std::string& code) {
    auto r = parse_source(with_var(decl));
    SPIKE_EXPECT(r.diag_output.find("[" + code + "]") != std::string::npos);
}

void test_declaration_errors() {
    expect_error("  v : vetor 1..5] de real\n", "E820");
    expect_error("  v : vetor[..5] de real\n", "E821");
    expect_error("  v : vetor[1.5..5] de real\n", "E821");
    expect_error("  v : vetor[1 5] de real\n", "E822");
    expect_error("  v : vetor[1..5 de real\n", "E823");
    expect_error("  v : vetor[1..5] real\n", "E824");
    expect_error("  v : vetor[1..2, 1..2, 1..2, 1..2] de real\n", "E826");
    expect_error("  v : vetor[1..3 1..4] de real\n", "E827");
    expect_error("  v : vetor[1..3, 1..4 de real\n", "E823");
}

void test_declaration_error_skips_to_next_line() {
    auto r = parse_source(with_var("  v : vetor 1..5] de real\n  x : inteiro\n"));
    SPIKE_EXPECT(r.program.globals.size() == 1 && r.program.globals[0].name == "x");
}

void test_missing_bracket_in_use_is_e825() {
    auto r = parse_source("algoritmo \"t\"\nvar\n  v : vetor[1..3] de inteiro\ninicio\n"
                          "  v[1 <- 2\nfimalgoritmo\n");
    SPIKE_EXPECT(r.diag_output.find("[E825]") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find("']' esperado depois do índice de 'v'") != std::string::npos);
}

} // namespace

namespace spike_tests {

void run_parser_array_tests() {
    test_lexer_range_dots();
    test_array_declaration();
    test_matrix_declaration();
    test_matrix_use();
    test_array_use_in_expressions_and_targets();
    test_declaration_errors();
    test_declaration_error_skips_to_next_line();
    test_missing_bracket_in_use_is_e825();
}

} // namespace spike_tests
