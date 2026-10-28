#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/suggest.hpp"

#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {

using spike::suggest;

const std::vector<std::string> kCommands = {"escreva", "escreval", "leia",       "se",
                                            "enquanto", "para",     "repita",     "interrompa",
                                            "retorne",  "escolha"};

void test_distance() {
    SPIKE_EXPECT(spike::edit_distance("escreva", "escreav") == 1);
    SPIKE_EXPECT(spike::edit_distance("escreva", "escrva") == 1);
    SPIKE_EXPECT(spike::edit_distance("escreva", "escrevaa") == 1);
    SPIKE_EXPECT(spike::edit_distance("ESCREVA", "escreva") == 0);
    SPIKE_EXPECT(spike::edit_distance("MÉDIA", "média") == 0);
    SPIKE_EXPECT(spike::edit_distance("média", "media") == 1);
    SPIKE_EXPECT(spike::edit_distance("", "abc") == 3);
    SPIKE_EXPECT(spike::edit_distance("ab", "ba") == 1);
    SPIKE_EXPECT(spike::edit_distance("kitten", "sitting") == 3);
}

struct Case {
    std::string word;
    std::vector<std::string> candidates;
    std::optional<std::string> expected;
};

void test_suggestion_table() {
    const std::vector<Case> cases = {
        {"escreav", kCommands, "escreva"},
        {"escrva", kCommands, "escreva"},
        {"escrevall", kCommands, "escreval"},
        {"ESCRVA", kCommands, "escreva"},
        {"lei", kCommands, "leia"},
        {"contadr", {"contador", "conta"}, "contador"},
        {"mdia", {"média"}, "média"},
        {"média", {"media"}, "media"},
        {"abc", {"abd"}, "abd"},
        {"ab", {"ac"}, "ac"},
        {"a", {"b"}, std::nullopt},
        {"xyz", {"abc"}, std::nullopt},
        {"escreva", {"escreva"}, std::nullopt},
        {"Escreva", {"escreva"}, std::nullopt},
        {"cas", {"cass", "casa", "caso"}, "casa"},
        {"pato", {"patos", "pat"}, "pat"},
        {"abcdefghij", {"abcdefgxyz"}, "abcdefgxyz"},
        {"abcdefghij", {"abcdewxyzj"}, std::nullopt},
        {"abcdef", {"abcxyz"}, std::nullopt},
        {"fimenqanto", {"fimenquanto", "fimpara"}, "fimenquanto"},
        {"raix", {"raiz", "abs", "pot"}, "raiz"},
    };
    for (const Case& c : cases) {
        const std::optional<std::string> got = suggest(c.word, c.candidates);
        const bool ok = got == c.expected;
        if (!ok) {
            std::cerr << "  suggest('" << c.word << "') gave '" << got.value_or("<nada>")
                      << "', expected '" << c.expected.value_or("<nada>") << "'\n";
        }
        SPIKE_EXPECT(ok);
    }
}

void test_common_mistakes() {
    const std::vector<std::string> words = {
        "print", "printf", "cout",  "puts",   "println", "input",  "scanf",   "cin",
        "readline", "if",  "else",  "while",  "for",     "do",     "switch",  "break",
        "return", "function", "def", "void",  "int",     "integer", "long",   "float",
        "double", "string", "str",  "char",   "text",    "texto",  "bool",    "boolean",
        "true",   "false", "and",   "or",     "not"};
    for (const std::string& word : words) {
        const bool ok = spike::common_mistake(word).has_value();
        if (!ok) std::cerr << "  no common-mistake hint for '" << word << "'\n";
        SPIKE_EXPECT(ok);
    }
    SPIKE_EXPECT(spike::common_mistake("PRINT") == spike::common_mistake("print"));
    SPIKE_EXPECT(spike::common_mistake("print")->find("escreva()") != std::string::npos);
    SPIKE_EXPECT(spike::common_mistake("if")->find("se ... entao ... fimse") != std::string::npos);
    SPIKE_EXPECT(spike::common_mistake("true")->find("verdadeiro") != std::string::npos);
    SPIKE_EXPECT(!spike::common_mistake("escreva"));
    SPIKE_EXPECT(!spike::common_mistake("contador"));
}

void test_hint_format() {
    SPIKE_EXPECT(spike::hint_with_suggestion("contadr", {"contador"}, "declare 'contadr' na seção var.") ==
                 "você quis dizer 'contador'? Declare 'contadr' na seção var.");
    SPIKE_EXPECT(spike::hint_with_suggestion("print", {"prints"}, "x") ==
                 "em Portugol use escreva() ou escreval().");
    SPIKE_EXPECT(spike::hint_with_suggestion("zzz", {"contador"}, "declare.") == "declare.");
}

void test_speed() {
    std::vector<std::string> candidates;
    unsigned seed = 12345;
    for (int i = 0; i < 10000; ++i) {
        std::string word;
        for (int j = 0; j < 20; ++j) {
            seed = seed * 1103515245u + 12345u;
            word += static_cast<char>('a' + (seed >> 16) % 26);
        }
        candidates.push_back(word);
    }
    const auto start = std::chrono::steady_clock::now();
    const std::optional<std::string> found = suggest("abcdefghijklmnopqrst", candidates);
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - start)
                        .count();
    if (ms >= 100) std::cerr << "  suggest over 10000 candidates took " << ms << " ms\n";
    SPIKE_EXPECT(ms < 100);
    (void)found;
}

} // namespace

namespace spike_tests {

void run_suggest_tests() {
    test_distance();
    test_suggestion_table();
    test_common_mistakes();
    test_hint_format();
    test_speed();
}

} // namespace spike_tests
