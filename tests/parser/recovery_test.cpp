#include "parser/parser_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/ast.hpp"
#include "spike/ast_printer.hpp"
#include "spike/lexer.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <string>
#include <variant>
#include <vector>

namespace {

using spike_tests::parse_source;

constexpr char kEscape = 27;

struct Reported {
    std::vector<std::string> codes;
    std::vector<std::string> positions;
    std::string output;
};

std::vector<std::string> collect(const std::string& text, const std::string& open, char close) {
    std::vector<std::string> found;
    std::size_t at = text.find(open);
    while (at != std::string::npos) {
        const std::size_t start = at + open.size();
        const std::size_t end = std::min(text.find(close, start), text.find(kEscape, start));
        found.push_back(text.substr(start, end - start));
        at = text.find(open, start);
    }
    return found;
}

Reported errors_of(const std::string& source) {
    auto r = parse_source(source);
    Reported out;
    out.output = r.diag_output;
    out.codes = collect(r.diag_output, "erro[", ']');
    out.positions = collect(r.diag_output, " --> <test>:", '\n');
    return out;
}

std::string program(const std::string& body) {
    return "algoritmo \"t\"\nvar\n  b, x, i : inteiro\ninicio\n" + body + "fimalgoritmo\n";
}

void test_missing_entao_and_value_are_two_errors() {
    const auto r = errors_of(program("  b <- 5\n  se b > 3\n    escreval(\"grande\")\n  fimse\n"
                                     "  x <-\n"));
    SPIKE_EXPECT((r.codes == std::vector<std::string>{"E400", "E201"}));
    SPIKE_EXPECT((r.positions == std::vector<std::string>{"6:11", "9:7"}));
}

void test_missing_value_keeps_next_statement() {
    auto r = parse_source(program("  x <-\n  escreval(b)\n"));
    SPIKE_EXPECT(r.program.body.size() == 2);
    const auto codes = collect(r.diag_output, "erro[", ']');
    SPIKE_EXPECT((codes == std::vector<std::string>{"E201"}));
    SPIKE_EXPECT(spike::print_ast(r.program).find("Error (5:7)") != std::string::npos);
}

void test_three_independent_errors_all_appear() {
    const auto r = errors_of(program("  x <- )\n  escreval 1\n  leia(5)\n  b <- 2\n"));
    SPIKE_EXPECT((r.codes == std::vector<std::string>{"E201", "E301", "E306"}));
    SPIKE_EXPECT((r.positions == std::vector<std::string>{"5:8", "6:12", "7:8"}));
}

void test_missing_faca_still_parses_body() {
    auto r = parse_source(program("  enquanto x < 3\n    x <- x + )\n  fimenquanto\n"
                                  "  para i de 1 ate 3\n    escreval(i +)\n  fimpara\n"));
    const auto codes = collect(r.diag_output, "erro[", ']');
    SPIKE_EXPECT((codes == std::vector<std::string>{"E500", "E201", "E505", "E201"}));
    SPIKE_EXPECT(r.program.body.size() == 2);
    if (r.program.body.size() != 2) return;
    const auto* loop = std::get_if<spike::EnquantoStmt>(&r.program.body[0]->node);
    const auto* para = std::get_if<spike::ParaStmt>(&r.program.body[1]->node);
    SPIKE_EXPECT(loop && loop->body.size() == 1);
    SPIKE_EXPECT(para && para->body.size() == 1);
}

void test_unclosed_block_is_one_error_at_its_start() {
    const auto se = errors_of(program("  se x > 1 entao\n    x <- 2\n"));
    SPIKE_EXPECT((se.codes == std::vector<std::string>{"E401"}));
    SPIKE_EXPECT((se.positions == std::vector<std::string>{"5:3"}));

    const auto nested = errors_of(program("  enquanto x < 3 faca\n    se x > 1 entao\n"
                                          "      x <- 2\n  fimenquanto\n"));
    SPIKE_EXPECT((nested.codes == std::vector<std::string>{"E401"}));
    SPIKE_EXPECT((nested.positions == std::vector<std::string>{"6:5"}));

    SPIKE_EXPECT((errors_of(program("  para i de 1 ate 3 faca\n    x <- i\n")).codes ==
                  std::vector<std::string>{"E506"}));
    SPIKE_EXPECT((errors_of(program("  repita\n    x <- 1\n")).codes ==
                  std::vector<std::string>{"E600"}));
    SPIKE_EXPECT((errors_of(program("  escolha x\n    caso 1\n      x <- 2\n")).codes ==
                  std::vector<std::string>{"E811"}));
}

void test_stray_closer_is_one_error() {
    const auto r = errors_of(program("  fimse\n  x <- 1\n"));
    SPIKE_EXPECT((r.codes == std::vector<std::string>{"E300"}));
}

void test_same_position_reports_once() {
    const auto r = errors_of(program("  escreva(+)\n"));
    SPIKE_EXPECT((r.codes == std::vector<std::string>{"E201"}));
}

void test_bad_line_is_skipped_until_next_line() {
    const auto r = errors_of(program("  42 x <- ) leia\n  x <- 1\n"));
    SPIKE_EXPECT((r.codes == std::vector<std::string>{"E300"}));
}

void test_missing_inicio_recovers() {
    SPIKE_EXPECT((errors_of("algoritmo \"t\"\nvar\n  x : inteiro\n  x <- 1\n  escreval(x)\n"
                            "fimalgoritmo\n")
                      .codes == std::vector<std::string>{"E102"}));
    SPIKE_EXPECT((errors_of("algoritmo \"t\"\n  x : inteiro\ninicio\n  x <- 1\nfimalgoritmo\n")
                      .codes == std::vector<std::string>{"E102"}));
}

void test_missing_algoritmo_stops_with_one_error() {
    const auto r = errors_of("inicio\n  x <- )\n  escreval 1\nfimalgoritmo\n");
    SPIKE_EXPECT((r.codes == std::vector<std::string>{"E100"}));
}

void test_fimalgoritmo_is_only_checked_at_the_end() {
    const auto inside = errors_of(program("  se x > 1 entao\n    x <- 2\n"));
    SPIKE_EXPECT(inside.output.find("E103") == std::string::npos);
    const auto missing = errors_of("algoritmo \"t\"\ninicio\n  x <- 1\n");
    SPIKE_EXPECT((missing.codes == std::vector<std::string>{"E103"}));
}

void test_error_list_is_capped_at_twenty() {
    std::string body;
    for (int i = 0; i < 30; ++i) body += "  x <- )\n";
    const auto r = errors_of(program(body));
    SPIKE_EXPECT(r.codes.size() == 20);
    SPIKE_EXPECT(r.output.find("(mais 10 erros omitidos)") != std::string::npos);
}

std::string repeated(const std::string& piece, int times) {
    std::string out;
    for (int i = 0; i < times; ++i) out += piece;
    return out;
}

void test_absurd_programs_finish_fast() {
    const std::vector<std::string> sources = {
        "",
        "se se se se",
        program("se se se se\n"),
        "fimse",
        program("fimse\nfimse senao ate fimescolha\n"),
        "algoritmo inicio fimse senao fimenquanto ate caso para de faca fimalgoritmo",
        "algoritmo \"t\"\ninicio\n( ) [ ] <- := + - * / ^ , : .. = <> < >\nfimalgoritmo\n",
        program("  x <- " + repeated("(", 10000) + "\n"),
        program(repeated("  se x entao\n", 10000)),
        program("  x <- " + repeated("nao ", 10000) + "\n"),
        program("  x <- " + repeated("- ", 10000) + "1\n"),
        program("  x <- 2" + repeated(" ^ 2", 10000) + "\n"),
        program(repeated("fimse\n", 10000)),
        program(repeated("escolha x caso ", 3000)),
    };
    for (const std::string& source : sources) {
        const auto start = std::chrono::steady_clock::now();
        auto r = parse_source(source);
        const auto elapsed = std::chrono::steady_clock::now() - start;
        SPIKE_EXPECT(r.had_error);
        SPIKE_EXPECT(elapsed < std::chrono::seconds(2));
        SPIKE_EXPECT(collect(r.diag_output, "erro[", ']').size() <= 20);
    }
}

void test_deep_nesting_is_e202() {
    const auto r = errors_of(program("  x <- " + repeated("(", 10000) + "\n"));
    SPIKE_EXPECT((r.codes == std::vector<std::string>{"E202"}));
}

void test_lexer_reports_every_error() {
    const std::string source = "algoritmo \"t\"\ninicio\n  x <- 5 @\n  y <- 3 #\nfimalgoritmo\n";
    spike::Lexer lexer(source, "<test>");
    lexer.tokenize();
    SPIKE_EXPECT(lexer.diagnostics().error_count() == 2);
}

} // namespace

namespace spike_tests {

void run_parser_recovery_tests() {
    test_missing_entao_and_value_are_two_errors();
    test_missing_value_keeps_next_statement();
    test_three_independent_errors_all_appear();
    test_missing_faca_still_parses_body();
    test_unclosed_block_is_one_error_at_its_start();
    test_stray_closer_is_one_error();
    test_same_position_reports_once();
    test_bad_line_is_skipped_until_next_line();
    test_missing_inicio_recovers();
    test_missing_algoritmo_stops_with_one_error();
    test_fimalgoritmo_is_only_checked_at_the_end();
    test_error_list_is_capped_at_twenty();
    test_absurd_programs_finish_fast();
    test_deep_nesting_is_e202();
    test_lexer_reports_every_error();
}

} // namespace spike_tests
