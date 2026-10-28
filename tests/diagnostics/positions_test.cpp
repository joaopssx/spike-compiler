#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/lexer.hpp"
#include "spike/parser.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string strip_ansi(const std::string& text) {
    std::string out;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\033') {
            while (i < text.size() && text[i] != 'm') ++i;
            continue;
        }
        out.push_back(text[i]);
    }
    return out;
}

std::string diagnostics_of(const std::string& source) {
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    spike::Lexer lexer(source, "t.por");
    auto tokens = lexer.tokenize();
    if (lexer.had_error()) {
        lexer.diagnostics().print_all();
    } else {
        spike::Parser parser(std::move(tokens), "t.por", source);
        parser.parse();
        parser.diagnostics().print_all();
    }
    std::cerr.rdbuf(old);
    return strip_ansi(sink.str());
}

std::vector<std::string> lines_of(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) lines.push_back(line);
    return lines;
}

std::string caret_line(const std::string& out) {
    for (const std::string& line : lines_of(out)) {
        if (line.rfind("     | ", 0) == 0 && line.find('^') != std::string::npos) return line;
    }
    return "";
}

std::string shown_line(const std::string& out, int number) {
    const std::string prefix = (number < 10 ? "   " : "  ") + std::to_string(number) + " | ";
    for (const std::string& line : lines_of(out)) {
        if (line.rfind(prefix, 0) == 0) return line.substr(prefix.size());
    }
    return "";
}

std::string caret_at(std::size_t spaces) {
    return "     | " + std::string(spaces, ' ') + "^";
}

bool has(const std::string& out, const std::string& piece) {
    return out.find(piece) != std::string::npos;
}

const char* const kHeader = "algoritmo \"t\"\nvar x: inteiro\ninicio\n";

void test_caret_after_accented_letters() {
    const std::string out = diagnostics_of("inicio\n  escreval(\"Olá\") @\n");
    SPIKE_EXPECT(has(out, "t.por:2:19"));
    SPIKE_EXPECT(caret_line(out) == caret_at(18));
}

void test_caret_after_cedilla_tilde_and_acute() {
    const std::string out = diagnostics_of("inicio\n  escreval(\"ação é\") @\n");
    SPIKE_EXPECT(has(out, "t.por:2:22"));
    SPIKE_EXPECT(caret_line(out) == caret_at(21));
}

void test_accent_after_the_error_does_not_move_caret() {
    const std::string out = diagnostics_of("inicio\n  @ escreval(\"ação\")\n");
    SPIKE_EXPECT(has(out, "t.por:2:3"));
    SPIKE_EXPECT(caret_line(out) == caret_at(2));
}

void test_tab_is_expanded_and_caret_follows() {
    const std::string out = diagnostics_of("inicio\n\tx <- 1 @\n");
    SPIKE_EXPECT(has(out, "t.por:2:9"));
    SPIKE_EXPECT(shown_line(out, 2) == "    x <- 1 @");
    SPIKE_EXPECT(caret_line(out) == caret_at(11));
}

void test_tab_and_accent_together() {
    const std::string out = diagnostics_of("inicio\n\t\tescreval(\"é\")\t@\n");
    SPIKE_EXPECT(has(out, "t.por:2:17"));
    SPIKE_EXPECT(caret_line(out) == caret_at(25));
}

void test_e201_points_after_arrow_when_value_is_on_next_line() {
    const std::string out =
        diagnostics_of(std::string(kHeader) + "  x <-\n  escreval(x)\nfimalgoritmo\n");
    SPIKE_EXPECT(has(out, "erro[E201]"));
    SPIKE_EXPECT(has(out, "t.por:4:7"));
    SPIKE_EXPECT(caret_line(out) == caret_at(6));
}

void test_e400_points_after_condition_when_entao_is_missing() {
    const std::string out = diagnostics_of(std::string(kHeader) +
                                           "  se x > 1\n    x <- 2\n  fimse\nfimalgoritmo\n");
    SPIKE_EXPECT(has(out, "erro[E400]"));
    SPIKE_EXPECT(has(out, "t.por:4:11"));
    SPIKE_EXPECT(caret_line(out) == caret_at(10));
}

void test_e400_on_same_line_points_at_offending_token() {
    const std::string out = diagnostics_of(std::string(kHeader) +
                                           "  se x > 1 x <- 2\n  fimse\nfimalgoritmo\n");
    SPIKE_EXPECT(has(out, "t.por:4:12"));
}

void test_e302_after_string_with_accent_and_escape() {
    const std::string accent =
        diagnostics_of(std::string(kHeader) + "  escreval(\"Olá\"\nfimalgoritmo\n");
    SPIKE_EXPECT(has(accent, "erro[E302]"));
    SPIKE_EXPECT(has(accent, "t.por:4:17"));
    const std::string escape =
        diagnostics_of(std::string(kHeader) + "  escreval(\"a\\\"b\"\nfimalgoritmo\n");
    SPIKE_EXPECT(has(escape, "t.por:4:18"));
}

void test_e500_and_e505_on_next_line() {
    const std::string enquanto = diagnostics_of(
        std::string(kHeader) + "  enquanto x < 3\n    x <- x + 1\n  fimenquanto\nfimalgoritmo\n");
    SPIKE_EXPECT(has(enquanto, "erro[E500]"));
    SPIKE_EXPECT(has(enquanto, "t.por:4:17"));
    const std::string para = diagnostics_of(std::string(kHeader) +
                                            "  para x de 1 ate 3\n  fimpara\nfimalgoritmo\n");
    SPIKE_EXPECT(has(para, "erro[E505]"));
    SPIKE_EXPECT(has(para, "t.por:4:20"));
}

void test_e105_and_e802_on_next_line() {
    const std::string var = diagnostics_of("algoritmo \"t\"\nvar x\ninicio\nfimalgoritmo\n");
    SPIKE_EXPECT(has(var, "erro[E105]"));
    SPIKE_EXPECT(has(var, "t.por:2:6"));
    const std::string constant =
        diagnostics_of("algoritmo \"t\"\nconst\n  limite =\ninicio\nfimalgoritmo\n");
    SPIKE_EXPECT(has(constant, "erro[E802]"));
    SPIKE_EXPECT(has(constant, "t.por:3:11"));
}

void test_e813_after_caso_keyword() {
    const std::string out = diagnostics_of(std::string(kHeader) +
                                           "  escolha x\n  caso\n  fimescolha\nfimalgoritmo\n");
    SPIKE_EXPECT(has(out, "erro[E813]"));
    SPIKE_EXPECT(has(out, "t.por:5:7"));
}

void test_long_line_shows_window_with_caret_under_error() {
    std::string code = "  x <- 1";
    for (int i = 0; i < 30; ++i) code += " + 1";
    code += " @";
    for (int i = 0; i < 20; ++i) code += " + 2";
    const std::string out = diagnostics_of("inicio\n" + code + "\n");
    const std::string shown = shown_line(out, 2);
    SPIKE_EXPECT(shown.rfind("...", 0) == 0);
    SPIKE_EXPECT(shown.size() > 6 && shown.substr(shown.size() - 3) == "...");
    SPIKE_EXPECT(shown.size() == 86);
    const std::size_t at = shown.find('@');
    SPIKE_EXPECT(at != std::string::npos);
    SPIKE_EXPECT(caret_line(out) == caret_at(at));
}

void test_line_of_exactly_100_characters_is_not_cut() {
    const std::string code = std::string(98, ' ') + "@";
    const std::string out = diagnostics_of("inicio\n " + code + "\n");
    SPIKE_EXPECT(shown_line(out, 2) == " " + code);
    SPIKE_EXPECT(caret_line(out) == caret_at(99));
}

void test_invalid_utf8_reports_e005_at_first_bad_byte() {
    const std::string source = std::string("inicio\n  x <- \"é") + static_cast<char>(0xE9) + "\"\n";
    const std::string out = diagnostics_of(source);
    SPIKE_EXPECT(has(out, "erro[E005]: o arquivo não está em UTF-8 válido"));
    SPIKE_EXPECT(has(out, "t.por:2:10"));
    SPIKE_EXPECT(has(out, "= dica: salve o arquivo como UTF-8."));
    SPIKE_EXPECT(shown_line(out, 2) == "  x <- \"é?\"");
    SPIKE_EXPECT(caret_line(out) == caret_at(9));
}

void test_accented_name_gets_one_error_with_hint() {
    const std::string out = diagnostics_of("inicio\n  média <- 1\n");
    SPIKE_EXPECT(has(out, "erro[E001]: caractere desconhecido 'é'"));
    SPIKE_EXPECT(has(out, "t.por:2:4"));
    SPIKE_EXPECT(has(out, "nomes de variáveis não podem ter acento, use 'media' em vez de 'média'"));
    SPIKE_EXPECT(caret_line(out) == caret_at(3));
    SPIKE_EXPECT(out.find("erro[") == out.rfind("erro["));
}

} // namespace

namespace spike_tests {

void run_diagnostic_position_tests() {
    test_caret_after_accented_letters();
    test_caret_after_cedilla_tilde_and_acute();
    test_accent_after_the_error_does_not_move_caret();
    test_tab_is_expanded_and_caret_follows();
    test_tab_and_accent_together();
    test_e201_points_after_arrow_when_value_is_on_next_line();
    test_e400_points_after_condition_when_entao_is_missing();
    test_e400_on_same_line_points_at_offending_token();
    test_e302_after_string_with_accent_and_escape();
    test_e500_and_e505_on_next_line();
    test_e105_and_e802_on_next_line();
    test_e813_after_caso_keyword();
    test_long_line_shows_window_with_caret_under_error();
    test_line_of_exactly_100_characters_is_not_cut();
    test_invalid_utf8_reports_e005_at_first_bad_byte();
    test_accented_name_gets_one_error_with_hint();
}

} // namespace spike_tests
