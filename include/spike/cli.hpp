#ifndef SPIKE_CLI_HPP
#define SPIKE_CLI_HPP

#include <string>

namespace spike {

enum class Command {
    COMPILE,   // spike arquivo.por [flags]
    TOKENS,    // spike tokens arquivo.por — lista os tokens (debug)
    HELP,      // spike --help
    VERSION,   // spike --version
    FETCH,     // spike fetch
    DEV,       // spike dev
    RANDOM,    // spike random [--basico|--desafio|--quiz]
    UNKNOWN,
};

struct CliArgs {
    Command command = Command::HELP;

    // Para COMPILE e TOKENS:
    std::string input_file;

    // Para COMPILE:
    std::string output_file;       // -o nome.exe
    bool flag_run      = false;    // --run: compila e executa
    bool flag_time     = false;    // --time: mostra tempo de cada fase
    bool flag_verbose  = false;    // --verbose: detalha cada etapa
    bool flag_keep_c   = false;    // --c: mantém o .c gerado
    bool flag_ast      = false;    // --ast: imprime a AST

    // Para RANDOM:
    std::string random_mode;       // "", "basico", "desafio", "quiz"

    // Se houve erro no parsing dos args:
    bool parse_error = false;
    std::string parse_error_msg;
};

// Parseia argc/argv e retorna a struct preenchida.
CliArgs parse_args(int argc, char* argv[]);

// Imprime o help completo e colorido (em stdout).
void print_help();

// Imprime a versão (em stdout).
void print_version();

} // namespace spike

#endif // SPIKE_CLI_HPP
