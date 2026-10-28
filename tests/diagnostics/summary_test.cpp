#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "driver/pipeline_internal.hpp"
#include "spike/colors.hpp"
#include "spike/diagnostics.hpp"
#include "spike/summary.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace {

void test_failure_summary_counts() {
    SPIKE_EXPECT(spike::failure_summary(1, 0) == "falhou: 1 erro");
    SPIKE_EXPECT(spike::failure_summary(3, 0) == "falhou: 3 erros");
    SPIKE_EXPECT(spike::failure_summary(2, 1) == "falhou: 2 erros e 1 aviso");
    SPIKE_EXPECT(spike::failure_summary(1, 4) == "falhou: 1 erro e 4 avisos");
}

void test_success_counts() {
    SPIKE_EXPECT(spike::success_counts(0, 1) == "0 erros, 1 aviso");
    SPIKE_EXPECT(spike::success_counts(0, 2) == "0 erros, 2 avisos");
}

void test_explain_suggestion() {
    SPIKE_EXPECT(spike::explain_suggestion("E400") ==
                 "Para entender um erro: spike --explain E400");
}

void test_first_error_code_is_the_earliest() {
    spike::Diagnostics d("a.por", "a\nb\nc\n");
    SPIKE_EXPECT(d.first_error_code().empty());
    d.warning("W001", "aviso", 1, 1);
    SPIKE_EXPECT(d.first_error_code().empty());
    d.error("E700", "depois", 3, 1);
    d.error("E400", "antes", 2, 5);
    d.error("E401", "mesma linha, coluna menor", 2, 2);
    SPIKE_EXPECT(d.first_error_code() == "E401");
}

std::string failure_text(const spike::pipeline_detail::Reporter& reporter) {
    const spike::ColorMode saved = spike::color_mode();
    spike::set_color_mode(spike::ColorMode::Never);
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    reporter.print_failure();
    std::cerr.rdbuf(old);
    spike::set_color_mode(saved);
    return sink.str();
}

void test_reporter_adds_warnings_to_ok_line() {
    spike::pipeline_detail::Reporter reporter(false, false);
    SPIKE_EXPECT(reporter.with_counts("gerado 'a'") == "gerado 'a'");
    spike::Diagnostics d("a.por", "x\n");
    d.warning("W001", "aviso", 1, 1);
    reporter.record(d);
    SPIKE_EXPECT(reporter.with_counts("gerado 'a'") == "gerado 'a', 0 erros, 1 aviso");
    SPIKE_EXPECT(failure_text(reporter).empty());
}

void test_reporter_failure_summary() {
    spike::pipeline_detail::Reporter reporter(false, false);
    spike::Diagnostics lexer("a.por", "a\nb\n");
    lexer.error("E400", "primeiro", 2, 1);
    lexer.warning("W001", "aviso", 1, 1);
    reporter.record(lexer);
    reporter.record_error("E902");
    SPIKE_EXPECT(failure_text(reporter) ==
                 "falhou: 2 erros e 1 aviso\nPara entender um erro: spike --explain E400\n");
}

void test_reporter_failure_without_known_code() {
    spike::pipeline_detail::Reporter reporter(false, false);
    reporter.record_error("");
    SPIKE_EXPECT(failure_text(reporter) == "falhou: 1 erro\n");
}

} // namespace

namespace spike_tests {

void run_summary_tests() {
    test_failure_summary_counts();
    test_success_counts();
    test_explain_suggestion();
    test_first_error_code_is_the_earliest();
    test_reporter_adds_warnings_to_ok_line();
    test_reporter_failure_summary();
    test_reporter_failure_without_known_code();
}

} // namespace spike_tests
