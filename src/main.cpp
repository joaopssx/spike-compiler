#include "spike/cli.hpp"
#include "spike/colors.hpp"
#include "spike/commands.hpp"
#include "spike/compile_pipeline.hpp"
#include "spike/console.hpp"
#include "spike/explain.hpp"
#include "spike/file_reader.hpp"
#include "spike/lexer.hpp"
#include "spike/parser.hpp"
#include "spike/platform.hpp"
#include "spike/setup.hpp"
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

// Le o arquivo e roda o lexer. Em caso de falha de leitura imprime o erro e
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

// Conta linhas "humanas": "" -> 0, "abc" -> 1, "abc\n" -> 1, "abc\ndef" -> 2.
std::size_t count_lines(const std::string& s) {
    if (s.empty()) return 0;
    std::size_t newlines = static_cast<std::size_t>(
        std::count(s.begin(), s.end(), '\n'));
    return newlines + (s.back() == '\n' ? 0 : 1);
}

// Direita-padded com espacos ate `width`. Nao trunca strings mais longas.
std::string pad_right(const std::string& s, std::size_t width) {
    if (s.size() >= width) return s;
    return s + std::string(width - s.size(), ' ');
}

int run_tokens(const spike::CliArgs& args) {
    auto lo = read_and_lex(args.input_file);
    if (!lo) return 1;

    if (lo->lexer->had_error()) {
        // Diagnostics primeiro pra o erro ficar visivel mesmo se o terminal
        // for pequeno e o token dump rolar pra fora da tela.
        lo->lexer->diagnostics().print_all();
        return 1;
    }

    constexpr std::size_t kTypeWidth = 14;

    // Largura da coluna [L:C] = maior posicao encontrada, pra TIPO e lexeme
    // ficarem em colunas alinhadas mesmo com linhas/colunas de muitos digitos.
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
            std::cout << spike::gray(idx.str(), spike::Stream::Out) << " ";
        }

        std::cout << spike::gray(pad_right(position, pos_width), spike::Stream::Out) << " "
                  << spike::cyan(type_str, spike::Stream::Out) << " "
                  << "\"" << t.lexeme << "\"\n";
        ++printed;
    }

    const std::size_t lines = count_lines(lo->source);
    std::ostringstream summary;
    summary << "— " << printed << " tokens encontrados em " << lines
            << (lines == 1 ? " linha" : " linhas");
    std::cout << spike::gray(summary.str(), spike::Stream::Out) << std::endl;

    // Smoke test do parser - so pra confirmar que ele aceita/rejeita.
    spike::Parser parser(lo->tokens, args.input_file, lo->source);
    parser.parse();
    if (parser.had_error()) {
        std::cout << spike::red("parse: erros encontrados.", spike::Stream::Out) << std::endl;
        parser.diagnostics().print_all();
    } else {
        std::cout << spike::green("parse: OK", spike::Stream::Out) << std::endl;
    }
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    spike::init_console();

    std::vector<std::string> arguments = spike::utf8_arguments(argc, argv);
    std::vector<char*> pointers;
    for (std::string& argument : arguments) pointers.push_back(argument.data());
    spike::CliArgs args = spike::parse_args(static_cast<int>(pointers.size()), pointers.data());
    spike::set_color_mode(args.color);

    if (args.parse_error) {
        std::cerr << spike::red("erro: " + args.parse_error_msg) << std::endl;
        std::cerr << spike::gray("tente 'spike --help'.") << std::endl;
        return 2;
    }

    switch (args.command) {
        case spike::Command::HELP:    spike::print_help();    return 0;
        case spike::Command::VERSION: spike::print_version(); return 0;
        case spike::Command::EXPLAIN:
            return spike::run_explain(args.explain_code, std::cout, std::cerr);
        case spike::Command::TOKENS:  return run_tokens(args);
        case spike::Command::COMPILE: return spike::run_compile(args);
        case spike::Command::CHECK:   return spike::run_check(args);
        case spike::Command::FETCH:   return spike::run_fetch(std::cout);
        case spike::Command::SETUP:   return spike::run_setup(args);
        case spike::Command::DEV:     return spike::run_dev(args);
        case spike::Command::RANDOM:  return spike::run_random(std::cerr);
        case spike::Command::UNKNOWN:
            // ja tratado pelo bloco parse_error acima; mantido por completude.
            spike::print_help();
            return 2;
    }
    return 0;
}
