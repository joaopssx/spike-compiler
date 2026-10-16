#include "parser/parser_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/ast.hpp"
#include "spike/token.hpp"

#include <string>
#include <variant>

namespace {

using spike_tests::parse_source;
using spike_tests::tokens_of;

std::string with_body(const std::string& body) {
    return "algoritmo \"t\"\nvar\n  x : inteiro\ninicio\n" + body + "fimalgoritmo\n";
}

const spike::EscolhaStmt* first_escolha(const spike_tests::ParseResult& r) {
    if (r.program.body.empty()) return nullptr;
    return std::get_if<spike::EscolhaStmt>(&r.program.body[0]->node);
}

void test_lexer_escolha_keywords() {
    const auto toks = tokens_of("escolha CASO outrocaso fimescolha casou");
    SPIKE_EXPECT(toks.size() == 5);
    if (toks.size() != 5) return;
    SPIKE_EXPECT(toks[0].type == spike::TokenType::ESCOLHA);
    SPIKE_EXPECT(toks[1].type == spike::TokenType::CASO);
    SPIKE_EXPECT(toks[2].type == spike::TokenType::OUTROCASO);
    SPIKE_EXPECT(toks[3].type == spike::TokenType::FIM_ESCOLHA);
    SPIKE_EXPECT(toks[4].type == spike::TokenType::IDENTIFIER);
}

void test_escolha_structure() {
    auto r = parse_source(with_body("  escolha x\n    caso 1\n      x <- 0\n    caso 2, 3\n"
                                    "    caso 4 ate 6, 9\n      x <- 1\n    outrocaso\n"
                                    "      x <- 2\n  fimescolha\n"));
    SPIKE_EXPECT(!r.had_error);
    const auto* s = first_escolha(r);
    SPIKE_EXPECT(s != nullptr);
    if (!s) return;
    SPIKE_EXPECT(s->line == 5 && s->col == 3);
    SPIKE_EXPECT(s->cases.size() == 3);
    SPIKE_EXPECT(s->cases[0].labels.size() == 1 && s->cases[0].body.size() == 1);
    SPIKE_EXPECT(s->cases[1].labels.size() == 2 && s->cases[1].body.empty());
    SPIKE_EXPECT(s->cases[2].labels.size() == 2);
    SPIKE_EXPECT(s->cases[2].labels[0].range_end != nullptr);
    SPIKE_EXPECT(s->cases[2].labels[1].range_end == nullptr);
    SPIKE_EXPECT(s->has_default && s->default_body.size() == 1);
}

void test_escolha_with_only_outrocaso_is_valid() {
    auto r = parse_source(with_body("  escolha x\n    outrocaso\n      x <- 1\n  fimescolha\n"));
    SPIKE_EXPECT(!r.had_error);
    const auto* s = first_escolha(r);
    SPIKE_EXPECT(s && s->cases.empty() && s->has_default);
}

void test_missing_caso_is_e810() {
    auto r = parse_source(with_body("  escolha x\n    x <- 1\n    caso 1\n  fimescolha\n"));
    SPIKE_EXPECT(r.diag_output.find("[E810]") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find(":6:5") != std::string::npos);
}

void test_missing_fimescolha_is_e811() {
    auto r = parse_source(with_body("  escolha x\n    caso 1\n      x <- 1\n"));
    SPIKE_EXPECT(r.diag_output.find("[E811]") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find("E103") == std::string::npos);
}

void test_outrocaso_twice_or_before_caso_is_e812() {
    auto twice = parse_source(with_body("  escolha x\n    outrocaso\n    outrocaso\n  fimescolha\n"));
    SPIKE_EXPECT(twice.diag_output.find("'outrocaso' repetido") != std::string::npos);
    auto before = parse_source(with_body("  escolha x\n    outrocaso\n    caso 1\n  fimescolha\n"));
    SPIKE_EXPECT(before.diag_output.find("[E812]") != std::string::npos);
    SPIKE_EXPECT(before.diag_output.find("'caso' depois de 'outrocaso'") != std::string::npos);
}

void test_caso_without_value_is_e813() {
    auto r = parse_source(with_body("  escolha x\n    caso\n      x <- 1\n  fimescolha\n"));
    SPIKE_EXPECT(r.diag_output.find("[E813]") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find(":6:9") != std::string::npos);
}

void test_escolha_without_cases_is_e814() {
    auto r = parse_source(with_body("  escolha x\n  fimescolha\n"));
    SPIKE_EXPECT(r.diag_output.find("[E814]") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find(":5:3") != std::string::npos);
}

} // namespace

namespace spike_tests {

void run_parser_escolha_tests() {
    test_lexer_escolha_keywords();
    test_escolha_structure();
    test_escolha_with_only_outrocaso_is_valid();
    test_missing_caso_is_e810();
    test_missing_fimescolha_is_e811();
    test_outrocaso_twice_or_before_caso_is_e812();
    test_caso_without_value_is_e813();
    test_escolha_without_cases_is_e814();
}

} // namespace spike_tests
