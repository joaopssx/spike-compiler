#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "commands/dev_dump.hpp"
#include "spike/commands.hpp"
#include "spike/lexer.hpp"
#include "spike/parser.hpp"
#include "spike/semantic.hpp"

#include <filesystem>
#include <sstream>
#include <string>

namespace {

bool contains(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

void test_dev_folder_sits_next_to_the_source() {
    const std::string folder = spike::dev_folder_for("examples/hello.por");
    SPIKE_EXPECT(std::filesystem::u8path(folder) ==
                 std::filesystem::u8path("examples") / "hello.dev");
    SPIKE_EXPECT(spike::dev_folder_for("media.por") == "media.dev");
}

void test_first_line_drops_the_rest_and_carriage_return() {
    SPIKE_EXPECT(spike::first_line("gcc 14.2.0\r\nCopyright\n") == "gcc 14.2.0");
    SPIKE_EXPECT(spike::first_line("clang 18") == "clang 18");
    SPIKE_EXPECT(spike::first_line("") == "");
}

void test_os_and_arch_are_never_empty() {
    SPIKE_EXPECT(!spike::os_description().empty());
    SPIKE_EXPECT(!spike::arch_name().empty());
}

void test_random_is_honest_about_being_planned() {
    std::ostringstream err;
    SPIKE_EXPECT(spike::run_random(err) == 2);
    SPIKE_EXPECT(contains(err.str(), "ainda não existe"));
    SPIKE_EXPECT(contains(err.str(), "v1.0"));
}

void test_dumps_describe_tokens_symbols_and_types() {
    const std::string source =
        "algoritmo \"t\"\nvar\n  x : inteiro\ninicio\n  x <- 2 + 3\n  escreva(x)\nfimalgoritmo\n";
    spike::Lexer lexer(source, "t.por");
    auto tokens = lexer.tokenize();
    const std::string token_dump = spike::dump_tokens(tokens);
    SPIKE_EXPECT(contains(token_dump, "[1:1]"));
    SPIKE_EXPECT(contains(token_dump, "\"algoritmo\""));

    spike::Parser parser(std::move(tokens), "t.por", source);
    spike::Program program = parser.parse();
    SPIKE_EXPECT(!parser.had_error());
    spike::SemanticAnalyzer semantic(program, "t.por", source);
    semantic.analyze();
    SPIKE_EXPECT(!semantic.had_error());

    const std::string symbols = spike::dump_symbols(semantic.symbols());
    SPIKE_EXPECT(contains(symbols, "x"));
    SPIKE_EXPECT(contains(symbols, "inteiro"));
    SPIKE_EXPECT(contains(symbols, "3:3"));
    SPIKE_EXPECT(contains(symbols, "usada, atribuída"));

    const std::string types = spike::dump_expr_types(semantic.expr_types());
    SPIKE_EXPECT(contains(types, "inteiro     operação +"));
    SPIKE_EXPECT(contains(types, "número 2"));
    SPIKE_EXPECT(contains(types, "variável x"));
    SPIKE_EXPECT(types.find("número 2") < types.find("número 3"));
}

} // namespace

namespace spike_tests {

void run_commands_tests() {
    test_dev_folder_sits_next_to_the_source();
    test_first_line_drops_the_rest_and_carriage_return();
    test_os_and_arch_are_never_empty();
    test_random_is_honest_about_being_planned();
    test_dumps_describe_tokens_symbols_and_types();
}

} // namespace spike_tests
