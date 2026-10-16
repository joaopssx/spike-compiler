#include "codegen/codegen_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "codegen/codegen_internal.hpp"
#include "spike/ast.hpp"

#include <string>
#include <unordered_map>

namespace {

using spike_tests::contains;
using spike_tests::generate_body;
using spike_tests::generate_from_source;

std::size_t count_of(const std::string& text, const std::string& piece) {
    std::size_t n = 0;
    for (auto pos = text.find(piece); pos != std::string::npos; pos = text.find(piece, pos + 1)) {
        ++n;
    }
    return n;
}

void test_codegen_header_and_runtime() {
    const auto gen = generate_from_source("algoritmo \"t\"\ninicio\nfimalgoritmo\n", "ola.por");
    SPIKE_EXPECT(gen.front_end_ok);
    SPIKE_EXPECT(gen.result.ok);
    const std::string& c = gen.result.c_source;
    SPIKE_EXPECT(c.rfind("/* Gerado pelo Spike " SPIKE_VERSION " a partir de ola.por */\n\n", 0) ==
                 0);
    SPIKE_EXPECT(count_of(c, "#ifndef SPIKE_RUNTIME_CORE_H") == 1);
    SPIKE_EXPECT(contains(c, "int main(void) {\n    spike_init();\n    spike_flush();\n"
                             "    spike_cleanup();\n    return 0;\n}\n"));
    SPIKE_EXPECT(c.find('\r') == std::string::npos);
}

void test_codegen_globals_have_default_values() {
    const std::string c = generate_body("");
    SPIKE_EXPECT(contains(c, "spike_int v_a = 0LL;\nspike_int v_i = 0LL;\n"));
    SPIKE_EXPECT(contains(c, "spike_real v_r = 0.0;\n"));
    SPIKE_EXPECT(contains(c, "spike_str v_t = \"\";\n"));
    SPIKE_EXPECT(contains(c, "spike_bool v_b = 0;\n"));
}

void test_codegen_names_are_prefixed_and_lowercase() {
    const auto gen = generate_from_source(
        "algoritmo \"t\"\nvar\n  Idade, INT, main : inteiro\ninicio\n  IDADE <- int + Main\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(gen.result.ok);
    SPIKE_EXPECT(contains(gen.result.c_source, "spike_int v_idade = 0LL;"));
    SPIKE_EXPECT(contains(gen.result.c_source, "v_idade = spike_add_int(v_int, v_main);"));
}

void test_codegen_name_and_literal_helpers() {
    using namespace spike::codegen_detail;
    SPIKE_EXPECT(c_name("média") == "v_m_xc3_xa9dia");
    SPIKE_EXPECT(c_name("Total_2") == "v_total_2");
    SPIKE_EXPECT(int_literal(123.0) == "123LL");
    SPIKE_EXPECT(real_literal(0.1) == "0.1");
    SPIKE_EXPECT(real_literal(3.0) == "3.0");
    SPIKE_EXPECT(real_literal(2.5) == "2.5");
    SPIKE_EXPECT(string_literal("Olá \"mundo\"\n\t\\") == "\"Olá \\\"mundo\\\"\\n\\t\\\\\"");
    SPIKE_EXPECT(string_literal(std::string("?") + "?/") == "\"?\\?/\"");
    SPIKE_EXPECT(string_literal(std::string("a\x01" "b")) == "\"a\\001b\"");
    SPIKE_EXPECT(comment_safe("a*/b") == "a* /b");
}

void test_codegen_invalid_type_is_internal_error() {
    spike::Program program;
    program.globals.push_back(spike::VarDecl{"x", spike::TypeKind::Integer, nullptr, 2, 3, {}});
    program.body.push_back(spike::make_stmt(
        spike::AssignStmt{spike::make_expr(spike::VarExpr{"x", 4, 3}),
                          spike::make_expr(spike::NumberExpr{1.0, true, 4, 8}), 4, 3}));

    const std::unordered_map<const spike::Expr*, spike::TypeKind> no_types;
    const spike::ConstantMap no_constants;
    const auto missing = spike::generate_c(program, no_types, no_constants);
    SPIKE_EXPECT(!missing.ok);
    SPIKE_EXPECT(missing.error_code == "E900");
    SPIKE_EXPECT(missing.c_source.empty());
    SPIKE_EXPECT(missing.error_line == 4);

    const auto& value = std::get<spike::AssignStmt>(program.body[0]->node).value;
    const std::unordered_map<const spike::Expr*, spike::TypeKind> invalid = {
        {value.get(), spike::TypeKind::Invalid}};
    SPIKE_EXPECT(spike::generate_c(program, invalid, no_constants).error_code == "E900");
}

void test_codegen_supports_every_command() {
    const auto gen = generate_from_source(
        "algoritmo \"todos\"\n"
        "var\n  x : inteiro\n  t : caractere\ninicio\n"
        "  x <- 1\n"
        "  escreva(x)\n"
        "  escreval(t)\n"
        "  leia(x, t)\n"
        "  se x > 0 entao\n  senao\n  fimse\n"
        "  enquanto x < 3 faca\n    x <- x + 1\n  fimenquanto\n"
        "  para x de 1 ate 2 faca\n  fimpara\n"
        "  repita\n  ate verdadeiro\n"
        "fimalgoritmo\n");
    SPIKE_EXPECT(gen.front_end_ok);
    SPIKE_EXPECT(gen.result.ok);
    SPIKE_EXPECT(gen.result.error_code.empty());
}

} // namespace

namespace spike_tests {

void run_codegen_program_tests() {
    test_codegen_header_and_runtime();
    test_codegen_globals_have_default_values();
    test_codegen_names_are_prefixed_and_lowercase();
    test_codegen_name_and_literal_helpers();
    test_codegen_invalid_type_is_internal_error();
    test_codegen_supports_every_command();
}

} // namespace spike_tests
