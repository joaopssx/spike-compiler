#include "semantic/semantic_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <string>

namespace {

using spike::TypeKind;
using spike_tests::analyze_typed;
using spike_tests::check_expression;
using spike_tests::count_occurrences;
using spike_tests::kTypedVars;

std::string errors_of(const std::string& body, const std::string& extra = "") {
    return analyze_typed("algoritmo \"t\"\nvar\n" + std::string(kTypedVars) + extra + "inicio\n" +
                         body + "fimalgoritmo\n")
        ->diag_output;
}

bool single_error(const std::string& out, const std::string& code) {
    return count_occurrences(out, "erro[") == 1 && out.find("[" + code + "]") != std::string::npos;
}

bool contains(const std::string& out, const std::string& piece) {
    return out.find(piece) != std::string::npos;
}

void test_result_types() {
    SPIKE_EXPECT(check_expression("abs(i)").type == TypeKind::Integer);
    SPIKE_EXPECT(check_expression("abs(r)").type == TypeKind::Real);
    SPIKE_EXPECT(check_expression("raiz(i)").type == TypeKind::Real);
    SPIKE_EXPECT(check_expression("pot(2, r)").type == TypeKind::Real);
    SPIKE_EXPECT(check_expression("arredonda(r) + trunca(1) + piso(r) + teto(i)").type ==
                 TypeKind::Integer);
    SPIKE_EXPECT(check_expression("seno(r) + cosseno(1) + tangente(r)").type == TypeKind::Real);
    SPIKE_EXPECT(check_expression("log(i) * exp(r)").type == TypeKind::Real);
    SPIKE_EXPECT(check_expression("aleatorio(1, i)").type == TypeKind::Integer);
    SPIKE_EXPECT(check_expression("RAIZ(4) + Abs(-2)").type == TypeKind::Real);
    SPIKE_EXPECT(check_expression("abs(abs(i) - aleatorio(1, 6))").type == TypeKind::Integer);
}

void test_valid_uses() {
    SPIKE_EXPECT(errors_of("  i <- arredonda(r)\n  aleatorio(1, 2)\n  se raiz(i) > 2 entao\n"
                           "  fimse\n")
                     .empty());
    SPIKE_EXPECT(errors_of("  exp <- exp(1)\n", "  exp : real\n").empty());
}

void test_wrong_argument_count_is_e781() {
    const std::string out = errors_of("  r <- raiz(1, 2)\n");
    SPIKE_EXPECT(single_error(out, "E781"));
    SPIKE_EXPECT(contains(out, "'raiz' espera 1 argumento, recebeu 2"));
    SPIKE_EXPECT(contains(out, "use raiz(real): real."));
    SPIKE_EXPECT(contains(errors_of("  i <- aleatorio(6)\n"), "'aleatorio' espera 2 argumentos"));
}

void test_wrong_argument_type_is_e782() {
    const std::string out = errors_of("  r <- raiz(t)\n");
    SPIKE_EXPECT(single_error(out, "E782"));
    SPIKE_EXPECT(contains(out, "o argumento 1 de 'raiz' precisa ser 'real', mas é 'caractere'"));
    const std::string overloaded = errors_of("  r <- ABS(b)\n");
    SPIKE_EXPECT(contains(overloaded, "precisa ser 'inteiro' ou 'real', mas é 'logico'"));
    SPIKE_EXPECT(contains(overloaded, "use abs(inteiro): inteiro ou abs(real): real."));
    SPIKE_EXPECT(contains(errors_of("  i <- aleatorio(1, r)\n"), "o argumento 2 de 'aleatorio'"));
    SPIKE_EXPECT(single_error(errors_of("  i <- raiz(r)\n"), "E721"));
    SPIKE_EXPECT(single_error(errors_of("  i <- raiz(nada)\n"), "E700"));
}

void test_user_names_cannot_hide_builtins_e789() {
    const std::string out = errors_of("", "procedimento raiz()\ninicio\nfimprocedimento\n");
    SPIKE_EXPECT(single_error(out, "E789"));
    SPIKE_EXPECT(contains(out, "'raiz' é uma função pronta do Spike, escolha outro nome"));
    SPIKE_EXPECT(single_error(errors_of("", "funcao Abs(x: real): real\ninicio\n  retorne x\n"
                                            "fimfuncao\n"),
                              "E789"));
}

void test_builtin_as_value_is_e784() {
    const std::string out = errors_of("  r <- raiz\n");
    SPIKE_EXPECT(single_error(out, "E784"));
    SPIKE_EXPECT(contains(out, "'raiz' é uma função pronta, não uma variável"));
}

void test_builtins_are_not_constants() {
    auto out = analyze_typed("algoritmo \"t\"\nconst\n  X = raiz(4)\ninicio\nfimalgoritmo\n")
                   ->diag_output;
    SPIKE_EXPECT(single_error(out, "E741"));
    SPIKE_EXPECT(single_error(errors_of("  escolha i\n  caso abs(1)\n  fimescolha\n"), "E763"));
}

} // namespace

namespace spike_tests {

void run_semantic_builtin_tests() {
    test_result_types();
    test_valid_uses();
    test_wrong_argument_count_is_e781();
    test_wrong_argument_type_is_e782();
    test_user_names_cannot_hide_builtins_e789();
    test_builtin_as_value_is_e784();
    test_builtins_are_not_constants();
}

} // namespace spike_tests
