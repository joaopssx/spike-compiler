#include "semantic/semantic_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <string>

namespace {

using spike_tests::analyze_typed;

std::string errors_of(const std::string& subprograms, const std::string& body) {
    return analyze_typed("algoritmo \"t\"\nconst\n  LIMITE = 10\nvar\n  contador : inteiro\n" +
                         subprograms + "inicio\n" + body + "fimalgoritmo\n")
        ->diag_output;
}

bool has(const std::string& text, const std::string& piece) {
    return text.find(piece) != std::string::npos;
}

const char* kDobro = "funcao dobro(n: inteiro): inteiro\ninicio\n  retorne n * 2\nfimfuncao\n";

void test_undeclared_variable_suggestions() {
    SPIKE_EXPECT(has(errors_of("", "  escreval(contadr)\n"), "você quis dizer 'contador'?"));
    SPIKE_EXPECT(has(errors_of("", "  escreval(LIMTE)\n"), "você quis dizer 'LIMITE'?"));
    SPIKE_EXPECT(has(errors_of("", "  contador <- true\n"), "use verdadeiro e falso"));
    SPIKE_EXPECT(!has(errors_of("", "  escreval(zebra)\n"), "quis dizer"));
    const std::string local = "procedimento p()\nvar\n  soma : inteiro\ninicio\n  escreval(som)\n"
                              "fimprocedimento\n";
    SPIKE_EXPECT(has(errors_of(local, ""), "você quis dizer 'soma'?"));
    const std::string hidden = "procedimento p()\nvar\n  soma : inteiro\ninicio\n  soma <- 1\n"
                               "fimprocedimento\n";
    const std::string main_body = errors_of(hidden, "  escreval(som)\n");
    SPIKE_EXPECT(has(main_body, "[E700]") && !has(main_body, "quis dizer 'soma'"));
}

void test_undefined_subprogram_suggestions() {
    SPIKE_EXPECT(has(errors_of("", "  escrva(\"oi\")\n"), "você quis dizer 'escreva'?"));
    SPIKE_EXPECT(has(errors_of("", "  contador <- raix(4)\n"), "você quis dizer 'raiz'?"));
    SPIKE_EXPECT(has(errors_of(kDobro, "  contador <- dobor(2)\n"), "você quis dizer 'dobro'?"));
    SPIKE_EXPECT(has(errors_of(kDobro, "  dobor(2)\n"), "você quis dizer 'dobro'?"));
    SPIKE_EXPECT(has(errors_of("", "  print(\"oi\")\n"), "em Portugol use escreva() ou escreval()."));
    SPIKE_EXPECT(has(errors_of("", "  contador <- input()\n"), "use leia()"));
}

} // namespace

namespace spike_tests {

void run_semantic_suggest_tests() {
    test_undeclared_variable_suggestions();
    test_undefined_subprogram_suggestions();
}

} // namespace spike_tests
