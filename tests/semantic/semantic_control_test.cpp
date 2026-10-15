#include "semantic/semantic_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <string>

namespace {

using spike_tests::analyze_typed;
using spike_tests::count_occurrences;
using spike_tests::kTypedVars;
using spike_tests::program_with;

std::string errors_of(const std::string& body) {
    return analyze_typed(program_with(kTypedVars, body))->diag_output;
}

bool single_error(const std::string& out, const std::string& code) {
    return count_occurrences(out, "erro[") == 1 &&
           out.find("[" + code + "]") != std::string::npos;
}

void test_valid_conditions() {
    for (const std::string body : {
             "  se b entao\n  fimse\n",
             "  se i > 1 e r < 3 entao\n  senao\n  fimse\n",
             "  enquanto nao b faca\n    b <- verdadeiro\n  fimenquanto\n",
             "  repita\n    i <- i + 1\n  ate i >= 10\n",
             "  se t = u ou b entao\n  fimse\n",
         }) {
        SPIKE_EXPECT(errors_of(body).empty());
    }
}

void test_non_logical_condition_is_e730() {
    SPIKE_EXPECT(single_error(errors_of("  se 5 entao\n  fimse\n"), "E730"));
    SPIKE_EXPECT(single_error(errors_of("  enquanto t faca\n  fimenquanto\n"), "E730"));
    SPIKE_EXPECT(single_error(errors_of("  repita\n  ate r\n"), "E730"));
}

void test_condition_message_names_the_command() {
    SPIKE_EXPECT(errors_of("  se 5 entao\n  fimse\n")
                     .find("a condição do 'se' precisa ser logica, mas é 'inteiro'") !=
                 std::string::npos);
    SPIKE_EXPECT(errors_of("  enquanto t faca\n  fimenquanto\n").find("'enquanto'") !=
                 std::string::npos);
    SPIKE_EXPECT(errors_of("  repita\n  ate r\n").find("'repita'") != std::string::npos);
}

void test_condition_hint_for_integer_variable() {
    const std::string out = errors_of("  se i entao\n  fimse\n");
    SPIKE_EXPECT(out.find("'i <> 0'") != std::string::npos);
    SPIKE_EXPECT(out.find("'i = 1'") != std::string::npos);
    SPIKE_EXPECT(errors_of("  se 5 entao\n  fimse\n").find("compare com '='") !=
                 std::string::npos);
}

void test_condition_error_is_not_cascaded() {
    SPIKE_EXPECT(single_error(errors_of("  se t + 1 entao\n  fimse\n"), "E710"));
    SPIKE_EXPECT(single_error(errors_of("  se k entao\n  fimse\n"), "E700"));
}

void test_valid_para_loops() {
    for (const std::string body : {
             "  para i de 1 ate 10 faca\n  fimpara\n",
             "  para i de j ate j * 2 passo 2 faca\n    escreval(i)\n  fimpara\n",
             "  para i de 10 ate 1 passo -1 faca\n  fimpara\n",
         }) {
        SPIKE_EXPECT(errors_of(body).empty());
    }
}

void test_para_counter_must_be_integer() {
    const std::string out = errors_of("  para r de 1 ate 3 faca\n  fimpara\n");
    SPIKE_EXPECT(single_error(out, "E731"));
    SPIKE_EXPECT(out.find("mas 'r' é 'real'") != std::string::npos);
    SPIKE_EXPECT(out.find(":8:8") != std::string::npos);
}

void test_para_bounds_must_be_integer() {
    SPIKE_EXPECT(single_error(errors_of("  para i de 1.5 ate 3 faca\n  fimpara\n"), "E732"));
    SPIKE_EXPECT(single_error(errors_of("  para i de 1 ate r faca\n  fimpara\n"), "E732"));
    SPIKE_EXPECT(single_error(errors_of("  para i de 1 ate 9 passo 0.5 faca\n  fimpara\n"),
                              "E732"));
    SPIKE_EXPECT(single_error(errors_of("  para i de 1 ate t faca\n  fimpara\n"), "E732"));
    const std::string out = errors_of("  para i de 1 ate r faca\n  fimpara\n");
    SPIKE_EXPECT(out.find("o fim do 'para' precisa ser 'inteiro', mas é 'real'") !=
                 std::string::npos);
}

void test_literal_zero_step_is_e733() {
    SPIKE_EXPECT(single_error(errors_of("  para i de 1 ate 3 passo 0 faca\n  fimpara\n"),
                              "E733"));
    SPIKE_EXPECT(single_error(errors_of("  para i de 1 ate 3 passo -0 faca\n  fimpara\n"),
                              "E733"));
    SPIKE_EXPECT(errors_of("  para i de 1 ate 3 passo j faca\n  fimpara\n").empty());
}

void test_counter_write_inside_loop_is_recorded() {
    auto typed = analyze_typed(program_with(
        kTypedVars,
        "  para i de 1 ate 10 faca\n    i <- i + 1\n    leia(I)\n    j <- 2\n  fimpara\n"
        "  i <- 0\n"));
    SPIKE_EXPECT(typed->diag_output.empty());
    const auto& writes = typed->analyzer->loop_var_writes();
    SPIKE_EXPECT(writes.size() == 2);
    if (writes.size() == 2) {
        SPIKE_EXPECT(writes[0].name == "i");
        SPIKE_EXPECT(writes[0].line == 9);
        SPIKE_EXPECT(writes[1].name == "I");
        SPIKE_EXPECT(writes[1].line == 10);
    }
}

void test_nested_loop_with_same_counter_is_recorded() {
    auto typed = analyze_typed(program_with(
        kTypedVars, "  para i de 1 ate 3 faca\n    para i de 1 ate 2 faca\n    fimpara\n  fimpara\n"));
    SPIKE_EXPECT(typed->analyzer->loop_var_writes().size() == 1);
}

void test_empty_bodies_are_valid() {
    SPIKE_EXPECT(errors_of("  se b entao\n  senao\n  fimse\n"
                           "  enquanto b faca\n  fimenquanto\n"
                           "  para i de 1 ate 2 faca\n  fimpara\n"
                           "  repita\n  ate b\n")
                     .empty());
}

} // namespace

namespace spike_tests {

void run_semantic_control_tests() {
    test_valid_conditions();
    test_non_logical_condition_is_e730();
    test_condition_message_names_the_command();
    test_condition_hint_for_integer_variable();
    test_condition_error_is_not_cascaded();
    test_valid_para_loops();
    test_para_counter_must_be_integer();
    test_para_bounds_must_be_integer();
    test_literal_zero_step_is_e733();
    test_counter_write_inside_loop_is_recorded();
    test_nested_loop_with_same_counter_is_recorded();
    test_empty_bodies_are_valid();
}

} // namespace spike_tests
