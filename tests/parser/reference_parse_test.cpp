#include "parser/parser_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/ast.hpp"
#include "spike/ast_printer.hpp"

#include <string>

namespace {

using spike_tests::parse_source;

std::string with_procedure(const std::string& procedure) {
    return "algoritmo \"t\"\n" + procedure + "inicio\nfimalgoritmo\n";
}

void test_var_marks_reference_parameters() {
    auto r = parse_source(with_procedure("procedimento troca(var a: inteiro, var b: inteiro)\n"
                                         "inicio\nfimprocedimento\n"));
    SPIKE_EXPECT(!r.had_error);
    if (r.program.procedures.size() != 1) return;
    const auto& params = r.program.procedures[0].params;
    SPIKE_EXPECT(params.size() == 2 && params[0].by_ref && params[1].by_ref);
    SPIKE_EXPECT(params.size() == 2 && params[1].name == "b" && params[1].col == 40);
}

void test_var_applies_to_its_group_only() {
    auto r = parse_source(with_procedure("procedimento p(var a, b: inteiro, c: real, var d: real)\n"
                                         "inicio\nfimprocedimento\n"));
    SPIKE_EXPECT(!r.had_error);
    if (r.program.procedures.size() != 1) return;
    const auto& params = r.program.procedures[0].params;
    SPIKE_EXPECT(params.size() == 4);
    if (params.size() != 4) return;
    SPIKE_EXPECT(params[0].by_ref && params[1].by_ref && !params[2].by_ref && params[3].by_ref);
}

void test_printer_shows_var() {
    const std::string ast = spike::print_ast(
        parse_source(with_procedure("funcao f(var a: inteiro, v: vetor[1..3] de real): real\n"
                                    "inicio\n  retorne 1\nfimfuncao\n"))
            .program);
    SPIKE_EXPECT(ast.find("    Param var a : inteiro (2:14)\n"
                          "    Param v : vetor[1..3] de real (2:26)\n") != std::string::npos);
}

void test_var_without_name_is_e831() {
    auto r = parse_source(with_procedure("procedimento p(var: inteiro)\ninicio\nfimprocedimento\n"));
    SPIKE_EXPECT(r.diag_output.find("[E831]") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find("nome do parâmetro esperado") != std::string::npos);
}

} // namespace

namespace spike_tests {

void run_parser_reference_tests() {
    test_var_marks_reference_parameters();
    test_var_applies_to_its_group_only();
    test_printer_shows_var();
    test_var_without_name_is_e831();
}

} // namespace spike_tests
