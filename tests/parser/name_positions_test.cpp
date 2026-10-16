#include "parser/parser_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/ast.hpp"

#include <variant>

namespace {

using spike_tests::parse_source;

void test_leia_records_each_name_position() {
    auto r = parse_source("algoritmo \"t\"\ninicio\n  leia(a,  bb)\nfimalgoritmo\n");
    const auto& leia = std::get<spike::LeiaStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(leia.targets.size() == 2);
    if (leia.targets.size() != 2) return;
    SPIKE_EXPECT(spike::target_variable(leia.targets[0])->line == 3);
    SPIKE_EXPECT(spike::target_variable(leia.targets[0])->col == 8);
    SPIKE_EXPECT(spike::target_variable(leia.targets[1])->col == 12);
}

void test_para_records_variable_position() {
    auto r = parse_source(
        "algoritmo \"t\"\ninicio\n  para cont de 1 ate 2 faca\n  fimpara\nfimalgoritmo\n");
    const auto& para = std::get<spike::ParaStmt>(r.program.body[0]->node);
    SPIKE_EXPECT(para.var_pos.line == 3);
    SPIKE_EXPECT(para.var_pos.col == 8);
    SPIKE_EXPECT(para.col == 3);
}

} // namespace

namespace spike_tests {

void run_parser_name_position_tests() {
    test_leia_records_each_name_position();
    test_para_records_variable_position();
}

} // namespace spike_tests
