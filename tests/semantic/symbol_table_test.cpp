#include "semantic/semantic_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/symbol_table.hpp"

#include <string>
#include <vector>

namespace {

using spike_tests::analyze_source;
using spike_tests::program_with;

void test_symbol_table_declare_and_lookup() {
    spike::SymbolTable table;
    SPIKE_EXPECT(table.declare("Nota", spike::TypeKind::Real, 3, 3));
    SPIKE_EXPECT(!table.declare("NOTA", spike::TypeKind::Integer, 4, 3));
    SPIKE_EXPECT(table.size() == 1);

    const spike::Symbol* s = table.lookup("nota");
    SPIKE_EXPECT(s != nullptr);
    SPIKE_EXPECT(s->name == "Nota");
    SPIKE_EXPECT(s->type == spike::TypeKind::Real);
    SPIKE_EXPECT(s->line == 3);
    SPIKE_EXPECT(table.lookup("outra") == nullptr);
}

void test_symbol_table_keeps_declaration_order() {
    auto r = analyze_source(program_with("  c, a : inteiro\n  b : logico\n", ""));
    std::vector<std::string> names;
    for (const spike::Symbol& s : r.symbols) names.push_back(s.name);
    SPIKE_EXPECT((names == std::vector<std::string>{"c", "a", "b"}));
}

void test_symbol_types_come_from_declarations() {
    auto r = analyze_source(program_with(
        "  i : inteiro\n  x : real\n  t : caractere\n  ok : logico\n", ""));
    SPIKE_EXPECT(r.symbols.lookup("i")->type == spike::TypeKind::Integer);
    SPIKE_EXPECT(r.symbols.lookup("x")->type == spike::TypeKind::Real);
    SPIKE_EXPECT(r.symbols.lookup("t")->type == spike::TypeKind::String);
    SPIKE_EXPECT(r.symbols.lookup("ok")->type == spike::TypeKind::Bool);
}

void test_symbol_flags_for_assignment_and_read() {
    auto r = analyze_source(program_with("  a, b, c : inteiro\n", "  a <- 1\n  b <- a\n"));
    SPIKE_EXPECT(r.symbols.lookup("a")->assigned);
    SPIKE_EXPECT(r.symbols.lookup("a")->used);
    SPIKE_EXPECT(r.symbols.lookup("b")->assigned);
    SPIKE_EXPECT(!r.symbols.lookup("b")->used);
    SPIKE_EXPECT(!r.symbols.lookup("c")->assigned);
    SPIKE_EXPECT(!r.symbols.lookup("c")->used);
}

void test_symbol_flags_for_leia_and_escreva() {
    auto r = analyze_source(program_with("  n, m : inteiro\n", "  leia(n)\n  escreval(m)\n"));
    SPIKE_EXPECT(r.symbols.lookup("n")->assigned);
    SPIKE_EXPECT(!r.symbols.lookup("n")->used);
    SPIKE_EXPECT(r.symbols.lookup("m")->used);
    SPIKE_EXPECT(!r.symbols.lookup("m")->assigned);
}

void test_symbol_flags_for_para() {
    auto r = analyze_source(program_with("  i, fim : inteiro\n",
                                         "  para i de 1 ate fim faca\n  fimpara\n"));
    SPIKE_EXPECT(r.symbols.lookup("i")->assigned);
    SPIKE_EXPECT(!r.symbols.lookup("i")->used);
    SPIKE_EXPECT(r.symbols.lookup("fim")->used);
}

void test_symbol_flags_for_conditions() {
    auto r = analyze_source(program_with(
        "  a, b, c : logico\n",
        "  se a entao\n  fimse\n  enquanto b faca\n  fimenquanto\n  repita\n  ate c\n"));
    SPIKE_EXPECT(r.symbols.lookup("a")->used);
    SPIKE_EXPECT(r.symbols.lookup("b")->used);
    SPIKE_EXPECT(r.symbols.lookup("c")->used);
}

} // namespace

namespace spike_tests {

void run_symbol_table_tests() {
    test_symbol_table_declare_and_lookup();
    test_symbol_table_keeps_declaration_order();
    test_symbol_types_come_from_declarations();
    test_symbol_flags_for_assignment_and_read();
    test_symbol_flags_for_leia_and_escreva();
    test_symbol_flags_for_para();
    test_symbol_flags_for_conditions();
}

} // namespace spike_tests
