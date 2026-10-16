#include "driver/pipeline_internal.hpp"
#include "spike/ast_printer.hpp"
#include "spike/colors.hpp"
#include "spike/diagnostics.hpp"
#include "spike/file_reader.hpp"
#include "spike/lexer.hpp"
#include "spike/parser.hpp"

#include <iostream>
#include <memory>
#include <string>

namespace spike::pipeline_detail {

namespace {

constexpr const char* kIssuesUrl = "https://github.com/joaopssx/spike-compiler/issues";

} // namespace

int analyze_file(const std::string& path, Analysis& analysis, Reporter& reporter,
                 bool parse_only) {
    reporter.step(1, "lendo arquivo '" + path + "'");
    const ReadResult file = read_file(path);
    if (!file.ok) {
        std::cerr << red(file.error) << std::endl;
        return kExitProgramError;
    }
    if (!file.error.empty()) std::cerr << yellow(file.error) << std::endl;
    analysis.path = path;
    analysis.source = file.content;

    const Stopwatch lex_time;
    Lexer lexer(analysis.source, path);
    auto tokens = lexer.tokenize();
    reporter.add_phase("léxico", lex_time.ms());
    reporter.step(2, "análise léxica: " +
                         count_text(tokens.empty() ? 0 : tokens.size() - 1, "token", "tokens"));
    if (lexer.had_error()) {
        lexer.diagnostics().print_all();
        return kExitProgramError;
    }

    const Stopwatch parse_time;
    Parser parser(std::move(tokens), path, analysis.source);
    analysis.program = parser.parse();
    reporter.add_phase("sintático", parse_time.ms());
    reporter.step(3, "análise sintática: " + count_text(count_statements(analysis.program),
                                                        "comando", "comandos"));
    if (parser.had_error()) {
        parser.diagnostics().print_all();
        return kExitProgramError;
    }
    if (parse_only) return kExitOk;

    const Stopwatch semantic_time;
    analysis.semantic = std::make_unique<SemanticAnalyzer>(analysis.program, path, analysis.source);
    analysis.semantic->analyze();
    reporter.add_phase("semântico", semantic_time.ms());
    reporter.step(4, "análise semântica: " + count_text(analysis.semantic->symbols().size(),
                                                        "variável", "variáveis"));
    const Diagnostics& diagnostics = analysis.semantic->diagnostics();
    if (diagnostics.has_errors() || diagnostics.warning_count() > 0) diagnostics.print_all();
    return diagnostics.has_errors() ? kExitProgramError : kExitOk;
}

void print_driver_error(const DriverError& error) {
    std::cerr << red(bold("erro[" + error.code + "]:")) << " " << error.message << "\n";
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

int report_codegen_error(const Analysis& analysis, const CodegenResult& result) {
    const bool internal = result.error_code == "E900";
    Diagnostics diagnostics(analysis.path, analysis.source);
    diagnostics.error(result.error_code, result.error_message, result.error_line,
                      result.error_col,
                      internal ? std::string("isso é um defeito do Spike, não do seu programa. "
                                             "Abra uma issue em ") + kIssuesUrl + "."
                               : "esta versão do Spike ainda não transforma esse comando em C.");
    diagnostics.print_all();
    return internal ? kExitInternalError : kExitProgramError;
}

void print_success(const std::string& message) {
    std::cout << green(bold("ok:")) << " " << message << std::endl;
}

} // namespace spike::pipeline_detail
