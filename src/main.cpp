#include "spike/cli.hpp"
#include "spike/colors.hpp"
#include "spike/file_reader.hpp"
#include "spike/lexer.hpp"
#include "spike/parser.hpp"
#include "spike/token.hpp"

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct LexOutcome {
    std::string source;
    std::unique_ptr<spike::Lexer> lexer;
    std::vector<spike::Token> tokens;
};

// Lê o arquivo e roda o lexer. Em caso de falha de leitura imprime o erro e
// retorna nullptr; caller deve interpretar como exit code 1.
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
    out->source = read.content;
    out->lexer = std::make_unique<spike::Lexer>(out->source, path);
    out->tokens = out->lexer->tokenize();
    return out;
}

// Conta linhas "humanas": "" → 0, "abc" → 1, "abc\n" → 1, "abc\ndef" → 2.
std::size_t count_lines(const std::string& s) {
    if (s.empty()) return 0;
    std::size_t newlines = static_cast<std::size_t>(
        std::count(s.begin(), s.end(), '\n'));
    return newlines + (s.back() == '\n' ? 0 : 1);
}

// Direita-padded com espaços até `width`. Não trunca strings mais longas.
std::string pad_right(const std::string& s, std::size_t width) {
    if (s.size() >= width) return s;
    return s + std::string(width - s.size(), ' ');
}

int run_tokens(const spike::CliArgs& args) {
    auto lo = read_and_lex(args.input_file);
    if (!lo) return 1;

    if (lo->lexer->had_error()) {
        // Diagnostics primeiro pra o erro ficar visível mesmo se o terminal
        // for pequeno e o token dump rolar pra fora da tela.
        lo->lexer->diagnostics().print_all();
        return 1;
    }

    constexpr std::size_t kTypeWidth = 14;

    // Largura da coluna [L:C] = maior posição encontrada, pra TIPO e lexeme
    // ficarem em colunas alinhadas mesmo com linhas/colunas de muitos dígitos.
    std::size_t pos_width = 0;
    for (const spike::Token& t : lo->tokens) {
        if (t.type == spike::TokenType::END_OF_FILE) continue;
        const std::size_t w = std::string("[" + std::to_string(t.line) + ":" +
                                          std::to_string(t.col) + "]").size();
        if (w > pos_width) pos_width = w;
    }

    std::size_t printed = 0;
    for (const spike::Token& t : lo->tokens) {
        if (t.type == spike::TokenType::END_OF_FILE) continue;

        const std::string position =
            "[" + std::to_string(t.line) + ":" + std::to_string(t.col) + "]";
        const std::string type_str =
            pad_right(spike::token_type_to_string(t.type), kTypeWidth);

        if (args.flag_verbose) {
            std::ostringstream idx;
            idx << "#" << std::setw(3) << std::setfill('0') << (printed + 1);
            std::cout << spike::gray(idx.str()) << " ";
        }

        std::cout << spike::gray(pad_right(position, pos_width)) << " "
                  << spike::cyan(type_str) << " "
                  << "\"" << t.lexeme << "\"\n";
        ++printed;
    }

    const std::size_t lines = count_lines(lo->source);
    std::ostringstream summary;
    summary << "— " << printed << " tokens encontrados em " << lines
            << (lines == 1 ? " linha" : " linhas");
    std::cout << spike::gray(summary.str()) << std::endl;

    // Smoke test do parser — só pra confirmar que ele aceita/rejeita.
    // (PROMPT 014: estrutura do programa + var section. Statements e
    // expressões ainda não.)
    spike::Parser parser(lo->tokens, args.input_file, lo->source);
    parser.parse();
    if (parser.had_error()) {
        std::cout << spike::red("parse: erros encontrados.") << std::endl;
        parser.diagnostics().print_all();
    } else {
        std::cout << spike::green("parse: OK") << std::endl;
    }
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
        case spike::Command::TOKENS:  return run_tokens(args);
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
