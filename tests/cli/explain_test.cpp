#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/cli.hpp"
#include "spike/colors.hpp"
#include "spike/error_catalog.hpp"
#include "spike/explain.hpp"

#include <sstream>
#include <string>
#include <vector>

namespace {

struct Explained {
    int code;
    std::string out;
    std::string err;
};

Explained explain(const std::string& code) {
    const spike::ColorMode saved = spike::color_mode();
    spike::set_color_mode(spike::ColorMode::Never);
    std::ostringstream out;
    std::ostringstream err;
    const int status = spike::run_explain(code, out, err);
    spike::set_color_mode(saved);
    return Explained{status, out.str(), err.str()};
}

spike::CliArgs parse(const std::vector<std::string>& argv_in) {
    std::vector<char*> argv;
    for (const auto& s : argv_in) argv.push_back(const_cast<char*>(s.c_str()));
    return spike::parse_args(static_cast<int>(argv.size()), argv.data());
}

void test_catalog_is_sorted_and_complete() {
    const auto& entries = spike::all_errors();
    SPIKE_EXPECT(entries.size() > 100);
    for (std::size_t i = 0; i < entries.size(); ++i) {
        SPIKE_EXPECT(entries[i].code.size() == 4);
        SPIKE_EXPECT(!entries[i].title.empty());
        SPIKE_EXPECT(!entries[i].message.empty());
        SPIKE_EXPECT(!entries[i].when.empty());
        SPIKE_EXPECT(!entries[i].fix.empty());
        SPIKE_EXPECT(!entries[i].example.empty());
        if (i > 0) SPIKE_EXPECT(entries[i - 1].code < entries[i].code);
    }
}

void test_find_error_ignores_case() {
    const spike::ErrorEntry* upper = spike::find_error("E400");
    SPIKE_EXPECT(upper != nullptr);
    SPIKE_EXPECT(spike::find_error("e400") == upper);
    SPIKE_EXPECT(upper->title == "falta 'entao'");
    SPIKE_EXPECT(spike::find_error("E999") == nullptr);
    SPIKE_EXPECT(spike::find_error("") == nullptr);
}

void test_catalog_keeps_special_characters() {
    const spike::ErrorEntry* e002 = spike::find_error("E002");
    SPIKE_EXPECT(e002 != nullptr && e002->fix.find("\\n") != std::string_view::npos);
    const spike::ErrorEntry* e825 = spike::find_error("E825");
    SPIKE_EXPECT(e825 != nullptr && e825->title == "falta o ']' no uso do vetor");
}

void test_explain_prints_every_part() {
    const Explained e = explain("E400");
    SPIKE_EXPECT(e.code == 0);
    SPIKE_EXPECT(e.err.empty());
    SPIKE_EXPECT(e.out.rfind("E400 — falta 'entao'\n", 0) == 0);
    SPIKE_EXPECT(e.out.find("Mensagem: `'entao' esperado") != std::string::npos);
    SPIKE_EXPECT(e.out.find("Quando acontece: ") != std::string::npos);
    SPIKE_EXPECT(e.out.find("Como corrigir: ") != std::string::npos);
    SPIKE_EXPECT(e.out.find("Exemplo:\n    algoritmo \"exemplo\"") != std::string::npos);
    SPIKE_EXPECT(e.out.find("    se x > 1\n") != std::string::npos);
    SPIKE_EXPECT(e.out.find('\x1b') == std::string::npos);
}

void test_explain_accepts_lowercase() {
    SPIKE_EXPECT(explain("e400").out == explain("E400").out);
}

void test_explain_unknown_code() {
    const Explained e = explain("X12");
    SPIKE_EXPECT(e.code == 2);
    SPIKE_EXPECT(e.out.empty());
    SPIKE_EXPECT(e.err.find("não conheço o código X12; códigos válidos: E001-E005, E100-E105") !=
                 std::string::npos);
}

void test_explain_without_code_lists_all() {
    const Explained e = explain("");
    SPIKE_EXPECT(e.code == 0);
    for (const spike::ErrorEntry& entry : spike::all_errors()) {
        SPIKE_EXPECT(e.out.find("  " + std::string(entry.code) + "  ") != std::string::npos);
    }
    SPIKE_EXPECT(e.out.find("  E400  `'entao' esperado após condição do 'se'`\n") !=
                 std::string::npos);
}

void test_summarize_codes_collapses_ranges() {
    const std::string summary = spike::summarize_codes();
    SPIKE_EXPECT(summary.rfind("E001-E005, E100-E105, E200-E202, ", 0) == 0);
    SPIKE_EXPECT(summary.find(", E600, ") != std::string::npos);
}

void test_cli_explain() {
    const auto none = parse({"spike", "--explain"});
    SPIKE_EXPECT(none.command == spike::Command::EXPLAIN);
    SPIKE_EXPECT(!none.parse_error);
    SPIKE_EXPECT(none.explain_code.empty());
    SPIKE_EXPECT(parse({"spike", "--explain", "e400"}).explain_code == "e400");
    SPIKE_EXPECT(parse({"spike", "--explain", "E400", "E401"}).parse_error);
}

} // namespace

namespace spike_tests {

void run_explain_tests() {
    test_catalog_is_sorted_and_complete();
    test_find_error_ignores_case();
    test_catalog_keeps_special_characters();
    test_explain_prints_every_part();
    test_explain_accepts_lowercase();
    test_explain_unknown_code();
    test_explain_without_code_lists_all();
    test_summarize_codes_collapses_ranges();
    test_cli_explain();
}

} // namespace spike_tests
