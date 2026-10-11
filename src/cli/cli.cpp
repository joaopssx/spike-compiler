#include "spike/cli.hpp"

#include "spike/colors.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace spike {

namespace {

bool ends_with_por(const std::string& s) {
    if (s.size() < 4) return false;
    std::string tail = s.substr(s.size() - 4);
    std::transform(tail.begin(), tail.end(), tail.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return tail == ".por";
}

bool eq(const char* a, const char* b) {
    return std::strcmp(a, b) == 0;
}

// Parseia flags genéricas (de COMPILE) a partir de `start` em argv.
// Retorna true em sucesso, false (com parse_error preenchido) em erro.
bool parse_compile_flags(int argc, char* argv[], int start, CliArgs& out) {
    for (int i = start; i < argc; ++i) {
        const char* a = argv[i];
        if (eq(a, "-o")) {
            if (i + 1 >= argc) {
                out.parse_error = true;
                out.parse_error_msg =
                    "a flag '-o' precisa de um nome de arquivo depois dela.";
                return false;
            }
            const char* next = argv[i + 1];
            if (next[0] == '-' && next[1] != '\0') {
                out.parse_error = true;
                out.parse_error_msg =
                    std::string("a flag '-o' esperava um nome de arquivo, ")
                    + "mas recebeu outra flag: '" + next + "'.";
                return false;
            }
            out.output_file = next;
            ++i;
        } else if (eq(a, "--run")) {
            out.flag_run = true;
        } else if (eq(a, "--time")) {
            out.flag_time = true;
        } else if (eq(a, "--verbose")) {
            out.flag_verbose = true;
        } else if (eq(a, "--c")) {
            out.flag_keep_c = true;
        } else if (eq(a, "--ast")) {
            out.flag_ast = true;
        } else {
            out.parse_error = true;
            out.parse_error_msg =
                std::string("flag desconhecida: '") + a + "'.";
            return false;
        }
    }
    return true;
}

} // namespace

CliArgs parse_args(int argc, char* argv[]) {
    CliArgs out;

    if (argc <= 1) {
        out.command = Command::HELP;
        return out;
    }

    const char* a1 = argv[1];

    if (eq(a1, "--help") || eq(a1, "-h")) {
        out.command = Command::HELP;
        return out;
    }
    if (eq(a1, "--version") || eq(a1, "-v")) {
        out.command = Command::VERSION;
        return out;
    }
    if (eq(a1, "fetch")) {
        out.command = Command::FETCH;
        return out;
    }
    if (eq(a1, "dev")) {
        out.command = Command::DEV;
        return out;
    }
    if (eq(a1, "tokens")) {
        out.command = Command::TOKENS;
        if (argc < 3) {
            out.parse_error = true;
            out.parse_error_msg =
                "o comando 'tokens' precisa de um arquivo .por como argumento.";
            return out;
        }
        out.input_file = argv[2];
        // Aceita --verbose depois do arquivo (mesma flag do COMPILE, mesmo significado).
        for (int i = 3; i < argc; ++i) {
            if (eq(argv[i], "--verbose")) {
                out.flag_verbose = true;
            } else {
                out.parse_error = true;
                out.parse_error_msg =
                    std::string("flag desconhecida para 'tokens': '") +
                    argv[i] + "'.";
                return out;
            }
        }
        return out;
    }
    if (eq(a1, "random")) {
        out.command = Command::RANDOM;
        for (int i = 2; i < argc; ++i) {
            const char* a = argv[i];
            if (eq(a, "--basico"))  out.random_mode = "basico";
            else if (eq(a, "--desafio")) out.random_mode = "desafio";
            else if (eq(a, "--quiz"))    out.random_mode = "quiz";
            else {
                out.parse_error = true;
                out.parse_error_msg =
                    std::string("modo desconhecido para 'random': '") + a +
                    "'. use --basico, --desafio ou --quiz.";
                return out;
            }
        }
        return out;
    }

    // Caso default: arquivo .por para compilar.
    const std::string first = a1;
    if (ends_with_por(first)) {
        out.command = Command::COMPILE;
        out.input_file = first;
        parse_compile_flags(argc, argv, 2, out);
        return out;
    }

    out.command = Command::UNKNOWN;
    out.parse_error = true;
    out.parse_error_msg =
        std::string("comando ou arquivo nao reconhecido: '") + a1 +
        "'. tente 'spike --help'.";
    return out;
}

void print_version() {
    std::cout << caramel(std::string("Spike ") + SPIKE_VERSION) << "\n";
}

void print_help() {
    auto section = [](const std::string& title) {
        return bold(title);
    };
    auto item = [](const std::string& name, const std::string& desc) {
        // "  <name>   <desc>" com o nome em ciano
        return std::string("  ") + cyan(name) + "   " + desc;
    };

    std::cout
        << caramel("Spike Compiler") << " " << gray("—")
        << " compila Portugol para .exe\n"
        << "\n"
        << section("uso:") << "\n"
        << "  spike <arquivo.por> [flags]\n"
        << "  spike <comando>\n"
        << "\n"
        << section("flags de compilação:") << "\n"
        << item("-o <arquivo>", "nome do executável gerado (padrão: mesmo nome do .por)") << "\n"
        << item("--run       ", "compila e executa na hora") << "\n"
        << item("--time      ", "mostra quanto tempo cada etapa levou") << "\n"
        << item("--verbose   ", "mostra o que está fazendo em cada passo") << "\n"
        << item("--c         ", "salva o código C gerado (não apaga o .c)") << "\n"
        << item("--ast       ", "imprime a árvore sintática antes de compilar") << "\n"
        << "\n"
        << section("comandos:") << "\n"
        << item("tokens <arq>", "lista os tokens do arquivo (modo debug)") << "\n"
        << item("fetch       ", "info do sistema com o cachorro em ASCII") << "\n"
        << item("dev         ", "quem fez isso aqui") << "\n"
        << item("random      ", "gera um programa em Portugol aleatório") << "\n"
        << item("--help      ", "mostra isso aqui") << "\n"
        << item("--version   ", "versão do compilador") << "\n";
}

} // namespace spike
