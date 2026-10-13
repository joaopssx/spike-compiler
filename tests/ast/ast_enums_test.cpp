#include "parser/parser_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/ast.hpp"
#include "spike/ast_enums.hpp"

#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace {

using spike_tests::parse_expression;
using spike_tests::parse_source;

const std::vector<std::pair<spike::BinaryOp, std::string>> kBinaryOps = {
    {spike::BinaryOp::Add, "+"},  {spike::BinaryOp::Sub, "-"},
    {spike::BinaryOp::Mul, "*"},  {spike::BinaryOp::Div, "/"},
    {spike::BinaryOp::Eq, "="},   {spike::BinaryOp::Ne, "<>"},
    {spike::BinaryOp::Lt, "<"},   {spike::BinaryOp::Gt, ">"},
    {spike::BinaryOp::Le, "<="},  {spike::BinaryOp::Ge, ">="},
    {spike::BinaryOp::And, "e"},  {spike::BinaryOp::Or, "ou"},
};

const std::vector<std::pair<spike::TypeKind, std::string>> kTypeNames = {
    {spike::TypeKind::Integer, "inteiro"},
    {spike::TypeKind::Real, "real"},
    {spike::TypeKind::String, "caractere"},
    {spike::TypeKind::Bool, "logico"},
};

spike::TypeKind declared_type(const std::string& type_text) {
    auto r = parse_source("algoritmo \"t\"\nvar\n  x : " + type_text +
                          "\ninicio\nfimalgoritmo\n");
    if (r.program.globals.size() != 1) return spike::TypeKind::None;
    return r.program.globals[0].type;
}

void test_binary_op_to_string_all_values() {
    for (const auto& [op, text] : kBinaryOps) {
        SPIKE_EXPECT(spike::to_string(op) == text);
    }
}

void test_binary_op_round_trip_through_parser() {
    for (const auto& [op, text] : kBinaryOps) {
        auto r = parse_expression("a " + text + " b");
        SPIKE_EXPECT(!r.had_error);
        SPIKE_EXPECT(std::holds_alternative<spike::BinaryExpr>(r.expr->node));
        const auto& bin = std::get<spike::BinaryExpr>(r.expr->node);
        SPIKE_EXPECT(bin.op == op);
        SPIKE_EXPECT(spike::to_string(bin.op) == text);
    }
}

void test_unary_op_to_string_and_round_trip() {
    SPIKE_EXPECT(std::string(spike::to_string(spike::UnaryOp::Neg)) == "-");
    SPIKE_EXPECT(std::string(spike::to_string(spike::UnaryOp::Not)) == "nao");

    auto neg = parse_expression("-x");
    SPIKE_EXPECT(std::get<spike::UnaryExpr>(neg.expr->node).op == spike::UnaryOp::Neg);
    auto nao = parse_expression("NAO x");
    SPIKE_EXPECT(std::get<spike::UnaryExpr>(nao.expr->node).op == spike::UnaryOp::Not);
}

void test_type_name_all_values() {
    for (const auto& [type, text] : kTypeNames) {
        SPIKE_EXPECT(spike::type_name(type) == text);
    }
    SPIKE_EXPECT(std::string(spike::type_name(spike::TypeKind::None)) == "?");
    SPIKE_EXPECT(std::string(spike::type_name(spike::TypeKind::Invalid)) == "?");
}

void test_type_name_round_trip_through_parser() {
    for (const auto& [type, text] : kTypeNames) {
        SPIKE_EXPECT(declared_type(text) == type);
    }
}

void test_caracter_maps_to_string_type() {
    SPIKE_EXPECT(declared_type("caracter") == spike::TypeKind::String);
    SPIKE_EXPECT(declared_type("CARACTER") == spike::TypeKind::String);
}

void test_unknown_type_is_invalid_kind() {
    SPIKE_EXPECT(declared_type("numero") == spike::TypeKind::Invalid);
}

} // namespace

namespace spike_tests {

void run_ast_enum_tests() {
    test_binary_op_to_string_all_values();
    test_binary_op_round_trip_through_parser();
    test_unary_op_to_string_and_round_trip();
    test_type_name_all_values();
    test_type_name_round_trip_through_parser();
    test_caracter_maps_to_string_type();
    test_unknown_type_is_invalid_kind();
}

} // namespace spike_tests
