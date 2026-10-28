#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/diagnostics.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace {

// ---- diagnostics ----------------------------------------------------------

void test_diag_empty_starts_clean() {
    spike::Diagnostics d("foo.por", "linha 1\nlinha 2\n");
    SPIKE_EXPECT(d.has_errors() == false);
    SPIKE_EXPECT(d.error_count() == 0);
    SPIKE_EXPECT(d.warning_count() == 0);
}

void test_diag_error_counts() {
    spike::Diagnostics d("foo.por", "a\nb\nc\n");
    d.error("E001", "primeiro", 1, 1);
    d.error("E002", "segundo", 2, 1);
    d.warning("W001", "aviso", 3, 1);
    SPIKE_EXPECT(d.has_errors() == true);
    SPIKE_EXPECT(d.error_count() == 2);
    SPIKE_EXPECT(d.warning_count() == 1);
}

void test_diag_warning_only_does_not_set_has_errors() {
    spike::Diagnostics d("foo.por", "x\n");
    d.warning("W001", "so um aviso", 1, 1);
    SPIKE_EXPECT(d.has_errors() == false);
    SPIKE_EXPECT(d.warning_count() == 1);
}

void test_diag_handles_out_of_range_line() {
    // Linha 99 nao existe - nao deve crashar, source_line vira string vazia.
    spike::Diagnostics d("foo.por", "uma linha so\n");
    d.error("E999", "linha inexistente", 99, 1);
    SPIKE_EXPECT(d.error_count() == 1);
    // Smoke test: print_all nao deve crashar nem com linha fora do range.
    // (Nao verificamos saida - so que executa.)
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    d.print_all();
    std::cerr.rdbuf(old);
    SPIKE_EXPECT(!sink.str().empty());
}

void test_diag_print_contains_key_pieces() {
    spike::Diagnostics d("calc.por", "x <- 1\ny <- 2\nz <- 3\n");
    d.error("E042", "exemplo de erro", 2, 3, "use uma variavel valida");
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    d.print_all();
    std::cerr.rdbuf(old);
    const std::string out = sink.str();
    SPIKE_EXPECT(out.find("erro[E042]:") != std::string::npos);
    SPIKE_EXPECT(out.find("exemplo de erro") != std::string::npos);
    SPIKE_EXPECT(out.find("calc.por:2:3") != std::string::npos);
    SPIKE_EXPECT(out.find("y <- 2") != std::string::npos);          // a linha do erro
    SPIKE_EXPECT(out.find("x <- 1") != std::string::npos);          // contexto anterior
    SPIKE_EXPECT(out.find("z <- 3") != std::string::npos);          // contexto seguinte
    SPIKE_EXPECT(out.find("^") != std::string::npos);
    SPIKE_EXPECT(out.find("dica:") != std::string::npos);
    SPIKE_EXPECT(out.find("use uma variavel valida") != std::string::npos);
}

void test_diag_no_hint_omits_dica_line() {
    spike::Diagnostics d("a.por", "x\n");
    d.error("E001", "sem dica", 1, 1); // sem hint
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    d.print_all();
    std::cerr.rdbuf(old);
    SPIKE_EXPECT(sink.str().find("dica:") == std::string::npos);
}

void test_diag_prints_in_line_order() {
    spike::Diagnostics d("a.por", "um\ndois\ntres\nquatro\n");
    d.warning("W001", "quarta linha", 4, 1);
    d.error("E001", "segunda linha", 2, 5);
    d.warning("W003", "segunda linha antes", 2, 1);
    d.error("E002", "primeira linha", 1, 1);
    std::ostringstream sink;
    auto* old = std::cerr.rdbuf(sink.rdbuf());
    d.print_all();
    std::cerr.rdbuf(old);
    const std::string out = sink.str();
    const std::size_t first = out.find("[E002]");
    const std::size_t second = out.find("[W003]");
    const std::size_t third = out.find("[E001]");
    const std::size_t fourth = out.find("[W001]");
    SPIKE_EXPECT(fourth != std::string::npos);
    SPIKE_EXPECT(first < second && second < third && third < fourth);
}

} // namespace

namespace spike_tests {

void run_diagnostics_tests() {
    test_diag_empty_starts_clean();
    test_diag_error_counts();
    test_diag_warning_only_does_not_set_has_errors();
    test_diag_handles_out_of_range_line();
    test_diag_print_contains_key_pieces();
    test_diag_no_hint_omits_dica_line();
    test_diag_prints_in_line_order();
}

} // namespace spike_tests
