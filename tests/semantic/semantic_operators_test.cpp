#include "semantic/semantic_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace {

using spike::TypeKind;
using spike_tests::check_expression;
using spike_tests::count_occurrences;

struct Case {
    std::string expr;
    TypeKind expected;
    std::string code;
};

void expect_case(const Case& c) {
    const auto result = check_expression(c.expr);
    const bool type_ok = result.type == c.expected;
    const bool code_ok = c.code.empty()
                             ? result.diag_output.empty()
                             : count_occurrences(result.diag_output, "erro[") == 1 &&
                                   result.diag_output.find("[" + c.code + "]") != std::string::npos;
    if (!type_ok || !code_ok) {
        std::cerr << "  case: " << c.expr << " -> got '" << spike::type_name(result.type)
                  << "'\n" << result.diag_output;
    }
    SPIKE_EXPECT(type_ok);
    SPIKE_EXPECT(code_ok);
}

void test_literal_types() {
    for (const Case& c : std::vector<Case>{
             {"1", TypeKind::Integer, ""},
             {"1.5", TypeKind::Real, ""},
             {"\"a\"", TypeKind::String, ""},
             {"verdadeiro", TypeKind::Bool, ""},
             {"i", TypeKind::Integer, ""},
             {"t", TypeKind::String, ""},
         }) {
        expect_case(c);
    }
}

void test_arithmetic_operator_table() {
    for (const std::string op : {"+", "-", "*", "/"}) {
        const bool div = op == "/";
        expect_case({"i " + op + " j", div ? TypeKind::Real : TypeKind::Integer, ""});
        expect_case({"i " + op + " r", TypeKind::Real, ""});
        expect_case({"r " + op + " i", TypeKind::Real, ""});
        expect_case({"r " + op + " r", TypeKind::Real, ""});
        expect_case({"t " + op + " u", op == "+" ? TypeKind::String : TypeKind::Invalid,
                     op == "+" ? "" : "E710"});
        expect_case({"t " + op + " i", TypeKind::Invalid, "E710"});
        expect_case({"r " + op + " t", TypeKind::Invalid, "E710"});
        expect_case({"b " + op + " c", TypeKind::Invalid, "E710"});
        expect_case({"b " + op + " i", TypeKind::Invalid, "E710"});
    }
}

void test_integer_division_operators() {
    for (const std::string op : {"div", "mod"}) {
        expect_case({"i " + op + " j", TypeKind::Integer, ""});
        expect_case({"7 " + op + " 2", TypeKind::Integer, ""});
        expect_case({"i " + op + " r", TypeKind::Invalid, "E710"});
        expect_case({"r " + op + " i", TypeKind::Invalid, "E710"});
        expect_case({"t " + op + " i", TypeKind::Invalid, "E710"});
        expect_case({"b " + op + " c", TypeKind::Invalid, "E710"});
    }
    const auto real = check_expression("r div 2");
    SPIKE_EXPECT(real.diag_output.find("use '/'") != std::string::npos);
    SPIKE_EXPECT(real.diag_output.find("trunca()") != std::string::npos);
    const auto text = check_expression("t mod 2");
    SPIKE_EXPECT(text.diag_output.find("só funciona com números inteiros") != std::string::npos);
}

void test_power_is_always_real() {
    expect_case({"2 ^ 10", TypeKind::Real, ""});
    expect_case({"i ^ j", TypeKind::Real, ""});
    expect_case({"r ^ i", TypeKind::Real, ""});
    expect_case({"-i ^ 2", TypeKind::Real, ""});
    expect_case({"t ^ 2", TypeKind::Invalid, "E710"});
    expect_case({"b ^ 2", TypeKind::Invalid, "E710"});
}

void test_division_is_always_real() {
    expect_case({"7 / 2", TypeKind::Real, ""});
    expect_case({"(i + j) / 1", TypeKind::Real, ""});
}

void test_comparison_operator_table() {
    for (const std::string op : {"=", "<>", "<", ">", "<=", ">="}) {
        const bool equality = op == "=" || op == "<>";
        expect_case({"i " + op + " j", TypeKind::Bool, ""});
        expect_case({"i " + op + " r", TypeKind::Bool, ""});
        expect_case({"t " + op + " u", TypeKind::Bool, ""});
        expect_case({"b " + op + " c", equality ? TypeKind::Bool : TypeKind::Invalid,
                     equality ? "" : "E711"});
        expect_case({"t " + op + " i", TypeKind::Invalid, "E711"});
        expect_case({"b " + op + " i", TypeKind::Invalid, "E711"});
    }
}

void test_logical_operator_table() {
    for (const std::string op : {"e", "ou"}) {
        expect_case({"b " + op + " c", TypeKind::Bool, ""});
        expect_case({"b " + op + " (i > 0)", TypeKind::Bool, ""});
        expect_case({"i " + op + " b", TypeKind::Invalid, "E712"});
        expect_case({"b " + op + " t", TypeKind::Invalid, "E712"});
        expect_case({"r " + op + " r", TypeKind::Invalid, "E712"});
    }
}

void test_unary_operator_table() {
    expect_case({"nao b", TypeKind::Bool, ""});
    expect_case({"nao (i = 1)", TypeKind::Bool, ""});
    expect_case({"nao i", TypeKind::Invalid, "E713"});
    expect_case({"nao t", TypeKind::Invalid, "E713"});
    expect_case({"-i", TypeKind::Integer, ""});
    expect_case({"-r", TypeKind::Real, ""});
    expect_case({"-t", TypeKind::Invalid, "E713"});
    expect_case({"-b", TypeKind::Invalid, "E713"});
}

void test_type_error_messages() {
    auto plus = check_expression("t + 1");
    SPIKE_EXPECT(plus.diag_output.find(
                     "o operador '+' não funciona entre 'caractere' e 'inteiro'") !=
                 std::string::npos);
    SPIKE_EXPECT(plus.diag_output.find("paraTexto()") != std::string::npos);

    auto compare = check_expression("t < 1");
    SPIKE_EXPECT(compare.diag_output.find(
                     "não é possível comparar 'caractere' com 'inteiro' usando '<'") !=
                 std::string::npos);

    auto logical = check_expression("i e b");
    SPIKE_EXPECT(logical.diag_output.find("recebeu 'inteiro'") != std::string::npos);

    auto bools = check_expression("b < c");
    SPIKE_EXPECT(bools.diag_output.find("'=' e '<>'") != std::string::npos);
}

void test_type_error_points_at_operator() {
    auto r = check_expression("i + t");
    SPIKE_EXPECT(r.diag_output.find(":8:14") != std::string::npos);
}

void test_cascade_is_avoided() {
    auto nested = check_expression("((1 + \"a\") * 2) - i");
    SPIKE_EXPECT(nested.type == TypeKind::Invalid);
    SPIKE_EXPECT(count_occurrences(nested.diag_output, "erro[") == 1);

    auto undeclared = check_expression("k + \"a\"");
    SPIKE_EXPECT(count_occurrences(undeclared.diag_output, "erro[") == 1);
    SPIKE_EXPECT(undeclared.diag_output.find("E700") != std::string::npos);
}

} // namespace

namespace spike_tests {

void run_semantic_operator_tests() {
    test_literal_types();
    test_arithmetic_operator_table();
    test_division_is_always_real();
    test_integer_division_operators();
    test_power_is_always_real();
    test_comparison_operator_table();
    test_logical_operator_table();
    test_unary_operator_table();
    test_type_error_messages();
    test_type_error_points_at_operator();
    test_cascade_is_avoided();
}

} // namespace spike_tests
