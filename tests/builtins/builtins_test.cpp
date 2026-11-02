#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/builtins.hpp"
#include "spike/symbol_table.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

using spike::TypeKind;

void test_signatures_are_unique() {
    const auto& all = spike::all_builtins();
    SPIKE_EXPECT(all.size() == 28);
    for (std::size_t i = 0; i < all.size(); ++i) {
        SPIKE_EXPECT(!all[i].c_name.empty());
        const std::string key = spike::symbol_key(all[i].name);
        for (std::size_t j = i + 1; j < all.size(); ++j) {
            const bool same_name = key == spike::symbol_key(all[j].name);
            SPIKE_EXPECT(!(same_name && all[i].params == all[j].params));
            if (same_name) SPIKE_EXPECT(all[i].name == all[j].name);
        }
    }
}

void test_overloads_are_never_ambiguous() {
    for (const auto& sig : spike::all_builtins()) {
        SPIKE_EXPECT(spike::find_builtin(spike::symbol_key(sig.name), sig.params) == &sig);
    }
}

void test_exact_match_wins_over_conversion() {
    const auto* abs_int = spike::find_builtin("abs", {TypeKind::Integer});
    SPIKE_EXPECT(abs_int && abs_int->result == TypeKind::Integer);
    const auto* abs_real = spike::find_builtin("abs", {TypeKind::Real});
    SPIKE_EXPECT(abs_real && abs_real->result == TypeKind::Real);
    const auto* raiz = spike::find_builtin("raiz", {TypeKind::Integer});
    SPIKE_EXPECT(raiz && raiz->params[0] == TypeKind::Real);
    const auto* pot = spike::find_builtin("pot", {TypeKind::Integer, TypeKind::Real});
    SPIKE_EXPECT(pot && pot->c_name == "spike_pow");
}

void test_no_match() {
    SPIKE_EXPECT(spike::find_builtin("raiz", {TypeKind::String}) == nullptr);
    SPIKE_EXPECT(spike::find_builtin("raiz", {TypeKind::Real, TypeKind::Real}) == nullptr);
    SPIKE_EXPECT(spike::find_builtin("aleatorio", {TypeKind::Real, TypeKind::Integer}) == nullptr);
    SPIKE_EXPECT(spike::find_builtin("raizes", {TypeKind::Real}) == nullptr);
    SPIKE_EXPECT(spike::find_builtin("RAIZ", {TypeKind::Real}) == nullptr);
}

void test_names_ignore_case() {
    SPIKE_EXPECT(spike::is_builtin_name("RAIZ") && spike::is_builtin_name("Aleatorio"));
    SPIKE_EXPECT(!spike::is_builtin_name("fatorial"));
    SPIKE_EXPECT(spike::builtins_named("Abs").size() == 2);
    SPIKE_EXPECT(spike::builtins_named("PARATEXTO").size() == 3);
    const auto* text = spike::find_builtin("paratexto", {TypeKind::Integer});
    SPIKE_EXPECT(text && text->c_name == "spike_int_to_str");
}

void test_signature_text() {
    SPIKE_EXPECT(spike::signature_text(*spike::builtins_named("raiz")[0]) == "raiz(real): real");
    SPIKE_EXPECT(spike::signature_text(*spike::builtins_named("aleatorio")[0]) ==
                 "aleatorio(inteiro, inteiro): inteiro");
    SPIKE_EXPECT(spike::signature_text(*spike::builtins_named("ehnumero")[0]) ==
                 "ehNumero(caractere): logico");
    SPIKE_EXPECT(spike::signature_text(*spike::builtins_named("copia")[0]) ==
                 "copia(caractere, inteiro, inteiro): caractere");
}

void test_language_reference_lists_every_signature() {
    std::ifstream in(std::string(SPIKE_SOURCE_DIR) + "/docs/language.md", std::ios::binary);
    std::stringstream buffer;
    buffer << in.rdbuf();
    const std::string reference = buffer.str();
    SPIKE_EXPECT(!reference.empty());
    for (const auto& sig : spike::all_builtins()) {
        const std::string text = spike::signature_text(sig);
        const std::string call = "`" + text.substr(0, text.find("):") + 1) + "`";
        const bool listed = reference.find(call) != std::string::npos;
        if (!listed) std::cerr << "  docs/language.md does not list " << call << "\n";
        SPIKE_EXPECT(listed);
    }
}

} // namespace

namespace spike_tests {

void run_builtins_tests() {
    test_language_reference_lists_every_signature();
    test_signatures_are_unique();
    test_overloads_are_never_ambiguous();
    test_exact_match_wins_over_conversion();
    test_no_match();
    test_names_ignore_case();
    test_signature_text();
}

} // namespace spike_tests
