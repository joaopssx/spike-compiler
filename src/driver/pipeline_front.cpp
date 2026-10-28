#include "driver/pipeline_internal.hpp"
#include "spike/ast_printer.hpp"
#include "spike/colors.hpp"
#include "spike/diagnostics.hpp"
#include "spike/file_reader.hpp"
#include "spike/lexer.hpp"
#include "spike/parser.hpp"
#include "spike/warnings.hpp"

#include <cstdio>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace spike::pipeline_detail {

namespace {

constexpr const char* kIssuesUrl = "https://github.com/joaopssx/spike-compiler/issues";

std::size_t line_count(const std::string& text) {
    std::size_t lines = 0;
    for (char c : text) {
        if (c == '\n') ++lines;
    }
    return lines;
}

} // namespace

int read_phase(const std::string& path, Analysis& analysis, Reporter& reporter) {
    reporter.step(1, "lendo arquivo '" + path + "'");
    const ReadResult file = read_file(path);
    if (!file.ok) {
        std::cerr << red(file.error) << std::endl;
        reporter.record_error("");
        return kExitProgramError;
    }
    if (!file.error.empty()) std::cerr << yellow(file.error) << std::endl;
    analysis.path = path;
    analysis.source = file.content;
    return kExitOk;
}

int lex_phase(const Analysis& analysis, std::vector<Token>& tokens, Reporter& reporter) {
    const Stopwatch lex_time;
    Lexer lexer(analysis.source, analysis.path);
    tokens = lexer.tokenize();
    reporter.add_phase("léxico", lex_time.ms());
    reporter.step(2, "análise léxica: " +
                         count_text(tokens.empty() ? 0 : tokens.size() - 1, "token", "tokens"));
    if (!lexer.had_error()) return kExitOk;
    lexer.diagnostics().print_all();
    reporter.record(lexer.diagnostics());
    return kExitProgramError;
}

int parse_phase(Analysis& analysis, std::vector<Token> tokens, Reporter& reporter) {
    const Stopwatch parse_time;
    Parser parser(std::move(tokens), analysis.path, analysis.source);
    analysis.program = parser.parse();
    reporter.add_phase("sintático", parse_time.ms());
    reporter.step(3, "análise sintática: " + count_text(count_statements(analysis.program),
                                                        "comando", "comandos"));
    if (!parser.had_error()) return kExitOk;
    parser.diagnostics().print_all();
    reporter.record(parser.diagnostics());
    return kExitProgramError;
}

int semantic_phase(Analysis& analysis, Reporter& reporter, const WarningPolicy& warnings) {
    const Stopwatch semantic_time;
    analysis.semantic =
        std::make_unique<SemanticAnalyzer>(analysis.program, analysis.path, analysis.source);
    analysis.semantic->analyze();
    Diagnostics found_warnings(analysis.path, analysis.source);
    check_warnings(analysis.program, *analysis.semantic, found_warnings);
    reporter.add_phase("semântico", semantic_time.ms());
    reporter.step(4, "análise semântica: " + count_text(analysis.semantic->symbols().size(),
                                                        "variável", "variáveis"));
    const Diagnostics& diagnostics = analysis.semantic->diagnostics();
    if (diagnostics.has_errors()) {
        diagnostics.print_all();
        reporter.record(diagnostics);
        return kExitProgramError;
    }
    if (warnings.show || warnings.as_errors) {
        found_warnings.print_all();
        reporter.record(found_warnings);
    }
    if (warnings.as_errors && found_warnings.warning_count() > 0) {
        std::cerr << red(bold("erro:")) << " "
                  << count_text(static_cast<std::size_t>(found_warnings.warning_count()), "aviso",
                                "avisos")
                  << " e '--warnings-as-errors' está ligado, então o programa não foi compilado."
                  << std::endl;
        return kExitProgramError;
    }
    return kExitOk;
}

CodegenOptions codegen_options(const Analysis& analysis, bool line_directives) {
    CodegenOptions options;
    options.source = analysis.source;
    options.emit_line_directives = line_directives;
    options.c_file_name = std::filesystem::u8path(analysis.path).stem().u8string() + ".c";
    return options;
}

int codegen_phase(const Analysis& analysis, CodegenResult& generated, Reporter& reporter,
                  bool line_directives) {
    const Stopwatch codegen_time;
    const std::string source_name = std::filesystem::u8path(analysis.path).filename().u8string();
    generated = generate_c(analysis.program, analysis.semantic->expr_types(),
                           analysis.semantic->constants(), source_name,
                           codegen_options(analysis, line_directives));
    reporter.add_phase("geração de C", codegen_time.ms());
    if (!generated.ok) return report_codegen_error(analysis, generated, reporter);
    reporter.step(5, "gerando C: " + count_text(line_count(generated.c_source), "linha", "linhas"));
    return kExitOk;
}

int analyze_file(const std::string& path, Analysis& analysis, Reporter& reporter,
                 bool parse_only, const WarningPolicy& warnings) {
    int status = read_phase(path, analysis, reporter);
    if (status != kExitOk) return status;
    std::vector<Token> tokens;
    status = lex_phase(analysis, tokens, reporter);
    if (status != kExitOk) return status;
    status = parse_phase(analysis, std::move(tokens), reporter);
    if (status != kExitOk || parse_only) return status;
    return semantic_phase(analysis, reporter, warnings);
}

void print_driver_error(const DriverError& error, Reporter& reporter) {
    reporter.record_error(error.code);
    std::cerr << red(bold("erro[" + error.code + "]:")) << " " << error.message << "\n";
    if (!error.location.empty()) {
        char number[16];
        std::snprintf(number, sizeof number, "%4d", error.source_line);
        std::cerr << cyan(" --> " + error.location) << "\n";
        std::cerr << "     " << gray("|") << "\n";
        std::cerr << gray(number) << " " << gray("|") << " " << error.source_text << "\n";
        std::cerr << "     " << gray("|") << "\n";
    }
    std::size_t start = 0;
    while (start < error.details.size()) {
        std::size_t end = error.details.find('\n', start);
        if (end == std::string::npos) end = error.details.size();
        std::cerr << "     " << gray("|") << " " << error.details.substr(start, end - start)
                  << "\n";
        start = end + 1;
    }
    if (!error.hint.empty()) std::cerr << "     " << green("= dica:") << " " << error.hint << "\n";
    std::cerr << std::endl;
}

int report_codegen_error(const Analysis& analysis, const CodegenResult& result,
                         Reporter& reporter) {
    const bool internal = result.error_code == "E900";
    Diagnostics diagnostics(analysis.path, analysis.source);
    diagnostics.error(result.error_code, result.error_message, result.error_line,
                      result.error_col,
                      internal ? std::string("isso é um defeito do Spike, não do seu programa. "
                                             "Abra uma issue em ") + kIssuesUrl + "."
                               : "esta versão do Spike ainda não transforma esse comando em C.");
    diagnostics.print_all();
    reporter.record(diagnostics);
    return internal ? kExitInternalError : kExitProgramError;
}

void print_success(const std::string& message) {
    std::cerr << green(bold("ok:")) << " " << message << std::endl;
}

} // namespace spike::pipeline_detail
