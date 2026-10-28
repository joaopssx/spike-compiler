#include "semantic/semantic_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <string>

namespace {

using spike_tests::count_occurrences;
using spike_tests::program_with;
using spike_tests::warnings_of;

const char* kVars = "  x : inteiro\n  r : real\n";

std::string body_warnings(const std::string& body) {
    return warnings_of(program_with(kVars, "  x <- 1\n  r <- 1.0\n" + body + "  escreval(x, r)\n"));
}

bool only(const std::string& out, const std::string& code) {
    return count_occurrences(out, "aviso[") == 1 &&
           out.find("aviso[" + code + "]") != std::string::npos;
}

bool none(const std::string& out) {
    return out.find("aviso[") == std::string::npos;
}

void test_division_by_literal_zero_is_w007() {
    const std::string out = body_warnings("  r <- x / 0\n");
    SPIKE_EXPECT(only(out, "W007"));
    SPIKE_EXPECT(out.find("<test>:8:12") != std::string::npos);
    SPIKE_EXPECT(only(body_warnings("  x <- x div 0\n"), "W007"));
    SPIKE_EXPECT(only(body_warnings("  x <- x mod 0\n"), "W007"));
    SPIKE_EXPECT(only(body_warnings("  r <- r / 0.0\n"), "W007"));
    SPIKE_EXPECT(only(body_warnings("  escreval(1 + x div -0)\n"), "W007"));
}

void test_division_by_other_values_is_fine() {
    SPIKE_EXPECT(none(body_warnings("  r <- x / 2\n")));
    SPIKE_EXPECT(none(body_warnings("  x <- x div x\n")));
    SPIKE_EXPECT(none(body_warnings("  x <- 0 mod 3\n")));
    SPIKE_EXPECT(none(body_warnings("  x <- x * 0\n")));
}

void test_literal_conditions_are_w008() {
    SPIKE_EXPECT(only(body_warnings("  se verdadeiro entao\n    x <- 2\n  fimse\n"), "W008"));
    SPIKE_EXPECT(only(body_warnings("  se falso entao\n    x <- 2\n  fimse\n"), "W008"));
    SPIKE_EXPECT(only(body_warnings("  enquanto falso faca\n    x <- 2\n  fimenquanto\n"), "W008"));
    SPIKE_EXPECT(only(body_warnings("  repita\n    x <- 2\n  ate verdadeiro\n"), "W008"));
    const std::string forever = body_warnings("  enquanto verdadeiro faca\n    x <- 2\n  fimenquanto\n");
    SPIKE_EXPECT(only(forever, "W008"));
    SPIKE_EXPECT(forever.find("sempre verdadeira") != std::string::npos);
}

void test_infinite_loop_with_exit_is_fine() {
    SPIKE_EXPECT(none(body_warnings(
        "  enquanto verdadeiro faca\n    x <- x + 1\n    se x > 3 entao\n      interrompa\n"
        "    fimse\n  fimenquanto\n")));
    SPIKE_EXPECT(none(body_warnings(
        "  repita\n    x <- x + 1\n    se x > 3 entao\n      interrompa\n    fimse\n  ate falso\n")));
    const std::string src =
        "algoritmo \"t\"\nfuncao primeiro(): inteiro\nvar\n  i : inteiro\ninicio\n  i <- 0\n"
        "  enquanto verdadeiro faca\n    i <- i + 1\n    se i > 3 entao\n      retorne i\n"
        "    fimse\n  fimenquanto\n  retorne 0\nfimfuncao\ninicio\n  escreval(primeiro())\n"
        "fimalgoritmo\n";
    SPIKE_EXPECT(none(warnings_of(src)));
}

void test_break_of_inner_loop_does_not_end_outer() {
    const std::string out = body_warnings(
        "  enquanto verdadeiro faca\n    enquanto x < 3 faca\n      interrompa\n"
        "    fimenquanto\n  fimenquanto\n");
    SPIKE_EXPECT(only(out, "W008"));
}

void test_computed_conditions_are_fine() {
    SPIKE_EXPECT(none(body_warnings("  se x > 0 entao\n    x <- 2\n  fimse\n")));
    SPIKE_EXPECT(none(body_warnings("  enquanto x < 3 faca\n    x <- x + 1\n  fimenquanto\n")));
}

void test_code_after_break_is_w009() {
    const std::string out = body_warnings(
        "  enquanto x < 3 faca\n    interrompa\n    x <- x + 1\n  fimenquanto\n");
    SPIKE_EXPECT(only(out, "W009"));
    SPIKE_EXPECT(out.find("<test>:10:5") != std::string::npos);
    SPIKE_EXPECT(out.find("'interrompa' (linha 9)") != std::string::npos);
}

void test_code_after_return_is_w009_once() {
    const std::string src =
        "algoritmo \"t\"\nfuncao um(): inteiro\ninicio\n  retorne 1\n  escreval(\"a\")\n"
        "  escreval(\"b\")\nfimfuncao\ninicio\n  escreval(um())\nfimalgoritmo\n";
    const std::string out = warnings_of(src);
    SPIKE_EXPECT(only(out, "W009"));
    SPIKE_EXPECT(out.find("<test>:5:3") != std::string::npos);
}

void test_return_at_the_end_is_fine() {
    const std::string src =
        "algoritmo \"t\"\nprocedimento p(n: inteiro)\ninicio\n  se n > 0 entao\n    retorne\n"
        "  fimse\n  escreval(n)\nfimprocedimento\ninicio\n  p(1)\nfimalgoritmo\n";
    SPIKE_EXPECT(none(warnings_of(src)));
}

void test_warnings_come_out_in_line_order() {
    const std::string src =
        "algoritmo \"t\"\nfuncao f(n: inteiro): inteiro\ninicio\n  se n > 0 entao\n"
        "    retorne 1\n  fimse\nfimfuncao\nprocedimento p()\nvar\n  sobra : inteiro\ninicio\n"
        "  escreval(f(1) / 0)\nfimprocedimento\ninicio\n  p()\nfimalgoritmo\n";
    const std::string out = warnings_of(src);
    const std::size_t w006 = out.find("aviso[W006]");
    const std::size_t w001 = out.find("aviso[W001]");
    const std::size_t w007 = out.find("aviso[W007]");
    SPIKE_EXPECT(w006 != std::string::npos && w001 != std::string::npos);
    SPIKE_EXPECT(w007 != std::string::npos);
    SPIKE_EXPECT(w006 < w001 && w001 < w007);
}

} // namespace

namespace spike_tests {

void run_warning_flow_tests() {
    test_division_by_literal_zero_is_w007();
    test_division_by_other_values_is_fine();
    test_literal_conditions_are_w008();
    test_infinite_loop_with_exit_is_fine();
    test_break_of_inner_loop_does_not_end_outer();
    test_computed_conditions_are_fine();
    test_code_after_break_is_w009();
    test_code_after_return_is_w009_once();
    test_return_at_the_end_is_fine();
    test_warnings_come_out_in_line_order();
}

} // namespace spike_tests
