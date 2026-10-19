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

const char* kFatorial = "algoritmo \"f\"\n"
                        "funcao fatorial(n: inteiro): inteiro\n"
                        "inicio\n"
                        "  se n <= 1 entao\n"
                        "    retorne 1\n"
                        "  fimse\n"
                        "  retorne n * fatorial(n - 1)\n"
                        "fimfuncao\n"
                        "procedimento sai()\n"
                        "inicio\n"
                        "  retorne\n"
                        "fimprocedimento\n"
                        "inicio\n"
                        "  escreval(fatorial(5))\n"
                        "  fatorial(2)\n"
                        "fimalgoritmo\n";

std::string with_function(const std::string& function, const std::string& body = "") {
    return "algoritmo \"t\"\nvar\n  x : inteiro\n" + function + "inicio\n" + body +
           "fimalgoritmo\n";
}

bool has_error(const std::string& src, const std::string& code) {
    return parse_source(src).diag_output.find("[" + code + "]") != std::string::npos;
}

void test_lexer_function_keywords() {
    std::vector<TokenType> types;
    for (const auto& t : tokens_of("funcao FUNCAO funcoes fimfuncao retorne retornei")) {
        types.push_back(t.type);
    }
    SPIKE_EXPECT((types == std::vector<TokenType>{TokenType::FUNCAO, TokenType::FUNCAO,
                                                  TokenType::IDENTIFIER, TokenType::FIM_FUNCAO,
                                                  TokenType::RETORNE, TokenType::IDENTIFIER}));
}

void test_function_declaration() {
    auto r = parse_source(kFatorial);
    SPIKE_EXPECT(!r.had_error);
    SPIKE_EXPECT(r.program.procedures.size() == 2);
    if (r.program.procedures.size() != 2) return;
    const spike::FunctionDecl& fatorial = r.program.procedures[0];
    SPIKE_EXPECT(fatorial.is_function() && fatorial.return_type == spike::TypeKind::Integer);
    SPIKE_EXPECT(fatorial.name == "fatorial" && fatorial.line == 2 && fatorial.col == 8);
    SPIKE_EXPECT(fatorial.end_line == 8 && fatorial.body.size() == 2);
    SPIKE_EXPECT(!r.program.procedures[1].is_function());
}

void test_retorne_value_is_optional() {
    auto r = parse_source(kFatorial);
    if (r.program.procedures.size() != 2) return;
    const auto& last = r.program.procedures[0].body[1];
    const auto* with_value = std::get_if<spike::ReturnStmt>(&last->node);
    SPIKE_EXPECT(with_value && with_value->value && with_value->line == 7);
    const auto* bare = std::get_if<spike::ReturnStmt>(&r.program.procedures[1].body[0]->node);
    SPIKE_EXPECT(bare && !bare->value);
    auto next_line = parse_source(with_function("procedimento p()\ninicio\n  retorne\n"
                                                "  x <- 1\nfimprocedimento\n"));
    SPIKE_EXPECT(!next_line.had_error);
    if (next_line.program.procedures.size() == 1) {
        SPIKE_EXPECT(next_line.program.procedures[0].body.size() == 2);
    }
}

void test_call_inside_expression() {
    auto r = parse_source(kFatorial);
    if (r.program.body.size() != 2) return;
    const auto& escreva = std::get<spike::EscrevaStmt>(r.program.body[0]->node);
    const auto* call = std::get_if<spike::CallExpr>(&escreva.args[0]->node);
    SPIKE_EXPECT(call && call->name == "fatorial" && call->args.size() == 1);
    SPIKE_EXPECT(call && call->line == 14 && call->col == 12);
    SPIKE_EXPECT(std::holds_alternative<spike::CallStmt>(r.program.body[1]->node));
}

void test_printer_shows_functions() {
    const std::string ast = spike::print_ast(parse_source(kFatorial).program);
    SPIKE_EXPECT(ast.find("  Function fatorial : inteiro (2:8)\n"
                          "    Param n : inteiro (2:17)\n") != std::string::npos);
    SPIKE_EXPECT(ast.find("Retorne (7:3)\n      Binary * (7:13)\n") != std::string::npos);
    SPIKE_EXPECT(ast.find("        Call fatorial (7:15)\n") != std::string::npos);
    SPIKE_EXPECT(ast.find("  Procedure sai (9:14)\n    Retorne (11:3)\n") != std::string::npos);
}

void test_return_type_errors_are_e835() {
    SPIKE_EXPECT(has_error(with_function("funcao f(a: inteiro)\ninicio\nfimfuncao\n"), "E835"));
    SPIKE_EXPECT(has_error(with_function("funcao f(a: inteiro):\ninicio\nfimfuncao\n"), "E835"));
    SPIKE_EXPECT(has_error(with_function("funcao f(): texto\ninicio\nfimfuncao\n"), "E835"));
    const std::string vetor =
        parse_source(with_function("funcao f(): vetor[1..2] de inteiro\ninicio\nfimfuncao\n"))
            .diag_output;
    SPIKE_EXPECT(vetor.find("uma função ainda não pode devolver um vetor") != std::string::npos);
    SPIKE_EXPECT(has_error(with_function("funcao (a: inteiro): real\ninicio\nfimfuncao\n"),
                           "E830"));
    SPIKE_EXPECT(has_error(with_function("funcao f a: inteiro): real\ninicio\nfimfuncao\n"),
                           "E831"));
}

void test_header_error_keeps_function_kind() {
    auto r = parse_source(with_function("funcao f(a inteiro): real\ninicio\n  retorne 1\n"
                                        "fimfuncao\n",
                                        "  x <- 2\n"));
    SPIKE_EXPECT(r.program.procedures.size() == 1);
    if (r.program.procedures.size() == 1) SPIKE_EXPECT(r.program.procedures[0].is_function());
    SPIKE_EXPECT(r.program.body.size() == 1);
}

void test_missing_fimfuncao_is_e836() {
    auto r = parse_source(with_function("funcao f(): inteiro\ninicio\n  retorne 1\n", "  x <- 1\n"));
    SPIKE_EXPECT(r.diag_output.find("[E836]") != std::string::npos);
    SPIKE_EXPECT(r.diag_output.find("'fimfuncao' esperado para fechar 'f'") != std::string::npos);
    SPIKE_EXPECT(r.program.body.size() == 1);
    auto swapped = parse_source(with_function("funcao f(): inteiro\ninicio\n  retorne 1\n"
                                              "fimprocedimento\n",
                                              "  x <- 1\n"));
    SPIKE_EXPECT(swapped.diag_output.find("[E836]") != std::string::npos);
    SPIKE_EXPECT(swapped.program.body.size() == 1);
    SPIKE_EXPECT(has_error(with_function("procedimento p()\ninicio\nfimfuncao\n"), "E832"));
}

void test_function_after_main_inicio_is_e833() {
    auto r = parse_source(with_function("", "  x <- 1\nfuncao f(): inteiro\ninicio\n  retorne 2\n"
                                            "fimfuncao\n  x <- 3\n"));
    SPIKE_EXPECT(r.diag_output.find("função declarada depois do 'inicio' do programa") !=
                 std::string::npos);
    SPIKE_EXPECT(r.program.body.size() == 2);
}

void test_const_section_ends_at_subprograms() {
    auto r = parse_source("algoritmo \"t\"\nconst\n  X = 2\nfuncao f(): inteiro\ninicio\n"
                          "  retorne X\nfimfuncao\nprocedimento p()\ninicio\nfimprocedimento\n"
                          "inicio\nfimalgoritmo\n");
    SPIKE_EXPECT(!r.had_error);
    SPIKE_EXPECT(r.program.consts.size() == 1 && r.program.procedures.size() == 2);
}

void test_call_expression_without_closing_paren_is_e834() {
    auto r = parse_source(with_function("", "  x <- f(1, 2\n"));
    SPIKE_EXPECT(r.diag_output.find("[E834]") != std::string::npos);
}

} // namespace

namespace spike_tests {

void run_parser_function_tests() {
    test_lexer_function_keywords();
    test_function_declaration();
    test_retorne_value_is_optional();
    test_call_inside_expression();
    test_printer_shows_functions();
    test_return_type_errors_are_e835();
    test_header_error_keeps_function_kind();
    test_missing_fimfuncao_is_e836();
    test_function_after_main_inicio_is_e833();
    test_const_section_ends_at_subprograms();
    test_call_expression_without_closing_paren_is_e834();
}

} // namespace spike_tests
