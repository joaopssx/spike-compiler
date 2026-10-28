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

// Parseia flags genericas (de COMPILE) a partir de `start` em argv.
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
        } else if (eq(a, "--no-warnings")) {
            out.flag_no_warnings = true;
        } else if (eq(a, "--warnings-as-errors")) {
            out.flag_warnings_as_errors = true;
        } else {
            out.parse_error = true;
            out.parse_error_msg =
                std::string("flag desconhecida: '") + a + "'.";
            return false;
        }
    }
    return true;
}

bool is_color_flag(const char* a) {
    return std::strncmp(a, "--color", 7) == 0 && (a[7] == '=' || a[7] == '\0');
}

bool parse_color_flag(const char* a, CliArgs& out) {
    const std::string value = a[7] == '=' ? a + 8 : "";
    if (value == "auto") {
        out.color = ColorMode::Auto;
    } else if (value == "always") {
        out.color = ColorMode::Always;
    } else if (value == "never") {
        out.color = ColorMode::Never;
    } else {
        out.parse_error = true;
        out.parse_error_msg = std::string("a flag '--color' aceita auto, always ou never, ") +
                              "como em '--color=never' (recebeu '" + a + "').";
        return false;
    }
    return true;
}

void parse_command(int argc, char* argv[], CliArgs& out);

} // namespace

CliArgs parse_args(int argc, char* argv[]) {
    CliArgs out;
    std::vector<char*> rest;
    for (int i = 0; i < argc; ++i) {
        if (i > 0 && is_color_flag(argv[i])) {
            if (!parse_color_flag(argv[i], out)) return out;
            continue;
        }
        rest.push_back(argv[i]);
    }
    parse_command(static_cast<int>(rest.size()), rest.data(), out);
    return out;
}

namespace {

void parse_command(int argc, char* argv[], CliArgs& out) {
    if (argc <= 1) {
        out.command = Command::HELP;
        return;
    }

    const char* a1 = argv[1];

    if (eq(a1, "--help") || eq(a1, "-h")) {
        out.command = Command::HELP;
        return;
    }
    if (eq(a1, "--version") || eq(a1, "-v")) {
        out.command = Command::VERSION;
        return;
    }
    if (eq(a1, "--explain")) {
        out.command = Command::EXPLAIN;
        if (argc > 3) {
            out.parse_error = true;
            out.parse_error_msg =
                "o comando '--explain' recebe só um código, como em 'spike --explain E400'.";
            return;
        }
        if (argc == 3) out.explain_code = argv[2];
        return;
    }
    if (eq(a1, "fetch")) {
        out.command = Command::FETCH;
        return;
    }
    if (eq(a1, "dev")) {
        out.command = Command::DEV;
        if (argc != 3) {
            out.parse_error = true;
            out.parse_error_msg = argc < 3
                ? "o comando 'dev' precisa de um arquivo .por, como em 'spike dev programa.por'."
                : "o comando 'dev' recebe só o arquivo .por.";
            return;
        }
        out.input_file = argv[2];
        return;
    }
    if (eq(a1, "tokens")) {
        out.command = Command::TOKENS;
        if (argc < 3) {
            out.parse_error = true;
            out.parse_error_msg =
                "o comando 'tokens' precisa de um arquivo .por como argumento.";
            return;
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
                return;
            }
        }
        return;
    }
    if (eq(a1, "check")) {
        out.command = Command::CHECK;
        if (argc != 3) {
            out.parse_error = true;
            out.parse_error_msg = argc < 3
                ? "o comando 'check' precisa de um arquivo .por como argumento."
                : "o comando 'check' recebe só o arquivo .por.";
            return;
        }
        out.input_file = argv[2];
        return;
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
                return;
            }
        }
        return;
    }

    // Caso default: arquivo .por para compilar.
    const std::string first = a1;
    if (ends_with_por(first)) {
        out.command = Command::COMPILE;
        out.input_file = first;
        parse_compile_flags(argc, argv, 2, out);
        return;
    }

    out.command = Command::UNKNOWN;
    out.parse_error = true;
    out.parse_error_msg =
        std::string("comando ou arquivo nao reconhecido: '") + a1 +
        "'. tente 'spike --help'.";
}

} // namespace

void print_version() {
    std::cout << caramel(std::string("Spike ") + SPIKE_VERSION, Stream::Out) << "\n";
}

void print_help() {
    auto section = [](const std::string& title) {
        return bold(title, Stream::Out);
    };
    auto item = [](const std::string& name, const std::string& desc) {
        // "  <name>   <desc>" com o nome em ciano
        return std::string("  ") + cyan(name, Stream::Out) + "   " + desc;
    };

    std::cout
        << caramel("Spike Compiler", Stream::Out) << " " << gray("—", Stream::Out)
        << " compila Portugol para .exe\n"
        << "\n"
        << section("uso:") << "\n"
        << "  spike <arquivo.por> [flags]\n"
        << "  spike <comando>\n"
        << "\n"
        << section("flags de compilação:") << "\n"
        << item("-o <arquivo>", "onde gravar o executável (padrão: o nome do .por, na mesma pasta)") << "\n"
        << item("--run       ", "depois de compilar, executa o programa") << "\n"
        << item("--c         ", "guarda o código C gerado ao lado do executável (<nome>.c)") << "\n"
        << item("--time      ", "mostra quanto tempo cada etapa levou, em milissegundos") << "\n"
        << item("--verbose   ", "conta o que o compilador está fazendo em cada etapa") << "\n"
        << item("--ast       ", "mostra a árvore do programa e não compila") << "\n"
        << "\n"
        << section("avisos:") << "\n"
        << item("--no-warnings       ", "não mostra os avisos (aviso[W...])") << "\n"
        << item("--warnings-as-errors", "qualquer aviso faz a compilação falhar") << "\n"
        << "\n"
        << section("comandos:") << "\n"
        << item("check <arq> ", "só confere o programa, sem gerar executável") << "\n"
        << item("tokens <arq>", "lista os tokens do arquivo (modo debug)") << "\n"
        << item("dev <arq>   ", "roda todas as fases e grava tokens, árvore, símbolos, tipos, C e tempos em <arq>.dev/") << "\n"
        << item("fetch       ", "mostra o ambiente do Spike (cole a saída ao pedir ajuda)") << "\n"
        << item("--help      ", "mostra isso aqui") << "\n"
        << item("--version   ", "versão do compilador") << "\n"
        << item("--explain   ", "explica um código de erro: spike --explain E400 (sem código, lista todos)") << "\n"
        << "\n"
        << section("planejados (ainda não funcionam):") << "\n"
        << item("random      ", "gerador de programas aleatórios (v1.0)") << "\n"
        << "\n"
        << section("cores (valem com qualquer comando):") << "\n"
        << item("--color=auto  ", "padrão: cores só no terminal; NO_COLOR desliga, FORCE_COLOR liga") << "\n"
        << item("--color=always", "sempre com cores") << "\n"
        << item("--color=never ", "nunca com cores") << "\n";
}

} // namespace spike
