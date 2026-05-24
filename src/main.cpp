#include "spike/cli.hpp"
#include "spike/colors.hpp"
#include "spike/file_reader.hpp"
#include "spike/lexer.hpp"
#include "spike/token.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace {

struct LexOutcome {
    std::unique_ptr<spike::Lexer> lexer;
    std::vector<spike::Token> tokens;
};

// Lê o arquivo e roda o lexer. Em caso de falha de leitura imprime o erro e
// retorna nullopt; caller deve interpretar como exit code 1.
std::unique_ptr<LexOutcome> read_and_lex(const std::string& path) {
    spike::ReadResult read = spike::read_file(path);
    if (!read.ok) {
        std::cerr << spike::red(read.error) << std::endl;
        return nullptr;
    }
    if (!read.error.empty()) {
        std::cerr << spike::yellow(read.error) << std::endl;
    }

    auto out = std::make_unique<LexOutcome>();
    out->lexer = std::make_unique<spike::Lexer>(read.content, path);
    out->tokens = out->lexer->tokenize();
    return out;
}

int run_tokens(const std::string& path) {
    auto lo = read_and_lex(path);
    if (!lo) return 1;

    for (const spike::Token& t : lo->tokens) {
        std::cout << spike::gray("[" + std::to_string(t.line) + ":" +
                                 std::to_string(t.col) + "]")
                  << " " << spike::cyan(spike::token_type_to_string(t.type))
                  << " \"" << t.lexeme << "\"\n";
    }
    if (lo->lexer->had_error()) {
        lo->lexer->diagnostics().print_all();
        return 1;
    }
    std::cout << spike::green("Analise lexica concluida sem erros.") << std::endl;
    return 0;
}

int run_compile(const spike::CliArgs& args) {
    auto lo = read_and_lex(args.input_file);
    if (!lo) return 1;

    if (lo->lexer->had_error()) {
        lo->lexer->diagnostics().print_all();
        return 1;
    }

    // TODO(prompts seguintes): parser + codegen.
    std::cerr << spike::yellow(
                     "compilação ainda não implementada — só análise léxica por enquanto.")
              << std::endl;
    if (!args.output_file.empty()) {
        std::cerr << spike::gray("  -o " + args.output_file + " (ignorado)") << std::endl;
    }
    if (args.flag_run || args.flag_time || args.flag_verbose ||
        args.flag_keep_c || args.flag_ast) {
        std::cerr << spike::gray("  flags ainda não têm efeito.") << std::endl;
    }
    return 0;
}

int run_stub(const std::string& label) {
    std::cerr << spike::yellow("'" + label + "' ainda não implementado.") << std::endl;
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    spike::init_colors();

    spike::CliArgs args = spike::parse_args(argc, argv);

    if (args.parse_error) {
        std::cerr << spike::red("erro: " + args.parse_error_msg) << std::endl;
        std::cerr << spike::gray("tente 'spike --help'.") << std::endl;
        return 2;
    }

    switch (args.command) {
        case spike::Command::HELP:    spike::print_help();    return 0;
        case spike::Command::VERSION: spike::print_version(); return 0;
        case spike::Command::TOKENS:  return run_tokens(args.input_file);
        case spike::Command::COMPILE: return run_compile(args);
        case spike::Command::FETCH:   return run_stub("fetch");
        case spike::Command::DEV:     return run_stub("dev");
        case spike::Command::RANDOM: {
            std::string label = "random";
            if (!args.random_mode.empty()) label += " --" + args.random_mode;
            return run_stub(label);
        }
        case spike::Command::UNKNOWN:
            // já tratado pelo bloco parse_error acima; mantido por completude.
            spike::print_help();
            return 2;
    }
    return 0;
}
