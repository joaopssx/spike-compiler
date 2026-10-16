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

const char* kDemo = "algoritmo \"demo\"\n"
                    "var\n"
                    "  n : inteiro\n"
                    "procedimento saudar(nome: caractere, vezes: inteiro)\n"
                    "var\n"
                    "  i : inteiro\n"
                    "inicio\n"
                    "  escreval(\"Oi, \", nome)\n"
                    "fimprocedimento\n"
                    "procedimento linha()\n"
                    "inicio\n"
                    "fimprocedimento\n"
                    "inicio\n"
                    "  saudar(\"Ana\", 2)\n"
                    "  linha()\n"
                    "fimalgoritmo\n";

std::string with_procedure(const std::string& procedure, const std::string& body = "") {
    return "algoritmo \"t\"\nvar\n  x : inteiro\n" + procedure + "inicio\n" + body +
           "fimalgoritmo\n";
}

bool has_error(const std::string& src, const std::string& code) {
    return parse_source(src).diag_output.find("[" + code + "]") != std::string::npos;
}

void test_lexer_procedure_keywords() {
    std::vector<TokenType> types;
    for (const auto& t : tokens_of("procedimento PROCEDIMENTO procedimentos fimprocedimento")) {
        types.push_back(t.type);
    }
    SPIKE_EXPECT((types == std::vector<TokenType>{TokenType::PROCEDIMENTO, TokenType::PROCEDIMENTO,
                                                  TokenType::IDENTIFIER,
                                                  TokenType::FIM_PROCEDIMENTO}));
}

void test_procedure_declaration() {
    auto r = parse_source(kDemo);
    SPIKE_EXPECT(!r.had_error);
    SPIKE_EXPECT(r.program.procedures.size() == 2);
    if (r.program.procedures.size() != 2) return;
    const spike::ProcedureDecl& saudar = r.program.procedures[0];
    SPIKE_EXPECT(saudar.name == "saudar" && saudar.line == 4 && saudar.col == 14);
    SPIKE_EXPECT(saudar.params.size() == 2);
    if (saudar.params.size() == 2) {
        SPIKE_EXPECT(saudar.params[0].name == "nome" &&
                     saudar.params[0].type == spike::TypeKind::String);
        SPIKE_EXPECT(saudar.params[1].name == "vezes" && saudar.params[1].col == 38 &&
                     saudar.params[1].type == spike::TypeKind::Integer);
    }
    SPIKE_EXPECT(saudar.locals.size() == 1 && saudar.locals[0].name == "i");
    SPIKE_EXPECT(saudar.body.size() == 1);
    SPIKE_EXPECT(r.program.procedures[1].params.empty() && r.program.procedures[1].body.empty());
    SPIKE_EXPECT(r.program.globals.size() == 1 && r.program.body.size() == 2);
    SPIKE_EXPECT(spike::count_statements(r.program) == 3);
}

void test_call_statement() {
    auto r = parse_source(kDemo);
    if (r.program.body.size() != 2) return;
    const auto* call = std::get_if<spike::CallStmt>(&r.program.body[0]->node);
    SPIKE_EXPECT(call && call->name == "saudar" && call->args.size() == 2);
    SPIKE_EXPECT(call && call->line == 14 && call->col == 3);
    const auto* empty = std::get_if<spike::CallStmt>(&r.program.body[1]->node);
    SPIKE_EXPECT(empty && empty->args.empty());
}

void test_printer_shows_procedures() {
    const std::string ast = spike::print_ast(parse_source(kDemo).program);
    SPIKE_EXPECT(ast.find("  Procedure saudar (4:14)\n"
                          "    Param nome : caractere (4:21)\n"
                          "    Param vezes : inteiro (4:38)\n"
                          "    Var i : inteiro (6:3)\n"
                          "    Escreval (8:3)\n") != std::string::npos);
    SPIKE_EXPECT(ast.find("  Call saudar (14:3)\n    String \"Ana\"\n    Number 2\n") !=
                 std::string::npos);
}

void test_grouped_parameters_and_no_var_section() {
    auto r = parse_source("algoritmo \"t\"\nprocedimento p(a, b: inteiro, c: real)\ninicio\n"
                          "fimprocedimento\ninicio\n  p(1, 2, 3)\nfimalgoritmo\n");
    SPIKE_EXPECT(!r.had_error);
    if (r.program.procedures.size() != 1) return;
    const auto& params = r.program.procedures[0].params;
    SPIKE_EXPECT(params.size() == 3 && params[1].type == spike::TypeKind::Integer &&
                 params[2].type == spike::TypeKind::Real);
}

void test_header_errors() {
    SPIKE_EXPECT(has_error(with_procedure("procedimento (a: inteiro)\ninicio\nfimprocedimento\n"),
                           "E830"));
    SPIKE_EXPECT(has_error(with_procedure("procedimento p a: inteiro)\ninicio\nfimprocedimento\n"),
                           "E831"));
    SPIKE_EXPECT(has_error(with_procedure("procedimento p(a inteiro)\ninicio\nfimprocedimento\n"),
                           "E831"));
    SPIKE_EXPECT(has_error(with_procedure("procedimento p(a: )\ninicio\nfimprocedimento\n"),
                           "E831"));
    SPIKE_EXPECT(has_error(with_procedure("procedimento p(a: inteiro\ninicio\nfimprocedimento\n"),
                           "E831"));
    SPIKE_EXPECT(has_error(with_procedure("procedimento p(, a: real)\ninicio\nfimprocedimento\n"),
                           "E831"));
    const std::string vetor =
        parse_source(with_procedure("procedimento p(v: vetor[1..3] de inteiro)\ninicio\n"
                                    "fimprocedimento\n"))
            .diag_output;
    SPIKE_EXPECT(vetor.find("um parâmetro ainda não pode ser um vetor") != std::string::npos);
}

void test_header_error_keeps_parsing() {
    auto r = parse_source(with_procedure("procedimento p(a inteiro)\ninicio\n  escreval(1)\n"
                                         "fimprocedimento\n",
                                         "  p(1)\n"));
    SPIKE_EXPECT(r.program.procedures.size() == 1);
    if (r.program.procedures.size() == 1) SPIKE_EXPECT(r.program.procedures[0].body.size() == 1);
    SPIKE_EXPECT(r.program.body.size() == 1);
}

void test_missing_fimprocedimento_is_e832() {
    auto r = parse_source(with_procedure("procedimento p()\ninicio\n  escreval(1)\n", "  p()\n"));
    SPIKE_EXPECT(r.diag_output.find("[E832]") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find("'fimprocedimento' esperado para fechar 'p'") !=
                 std::string::npos);
    SPIKE_EXPECT(r.program.body.size() == 1);
}

void test_procedure_after_main_inicio_is_e833() {
    auto r = parse_source(with_procedure("", "  x <- 1\nprocedimento p()\ninicio\n  x <- 2\n"
                                             "fimprocedimento\n  x <- 3\n"));
    SPIKE_EXPECT(r.diag_output.find("[E833]") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find("[E300]") == std::string::npos);
    SPIKE_EXPECT(r.program.body.size() == 2);
}

void test_call_without_closing_paren_is_e834() {
    auto r = parse_source(with_procedure("", "  p(1, 2\n"));
    SPIKE_EXPECT(r.diag_output.find("[E834]") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find("')' esperado no fim da chamada de 'p'") != std::string::npos);
}

} // namespace

namespace spike_tests {

void run_parser_procedure_tests() {
    test_lexer_procedure_keywords();
    test_procedure_declaration();
    test_call_statement();
    test_printer_shows_procedures();
    test_grouped_parameters_and_no_var_section();
    test_header_errors();
    test_header_error_keeps_parsing();
    test_missing_fimprocedimento_is_e832();
    test_procedure_after_main_inicio_is_e833();
    test_call_without_closing_paren_is_e834();
}

} // namespace spike_tests
