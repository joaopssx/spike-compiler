#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/cli.hpp"

#include <string>
#include <vector>

namespace {

// ---- cli -------------------------------------------------------------------

// Helper para chamar parse_args com um vetor de strings (em vez de argv).
// const_cast é seguro aqui porque parse_args só lê argv.
spike::CliArgs parse(const std::vector<std::string>& argv_in) {
    std::vector<char*> argv;
    argv.reserve(argv_in.size());
    for (const auto& s : argv_in) {
        argv.push_back(const_cast<char*>(s.c_str()));
    }
    return spike::parse_args(static_cast<int>(argv.size()), argv.data());
}

void test_cli_no_args_is_help() {
    auto a = parse({"spike"});
    SPIKE_EXPECT(a.command == spike::Command::HELP);
    SPIKE_EXPECT(a.parse_error == false);
}

void test_cli_help_flag() {
    SPIKE_EXPECT(parse({"spike", "--help"}).command == spike::Command::HELP);
    SPIKE_EXPECT(parse({"spike", "-h"}).command == spike::Command::HELP);
}

void test_cli_version_flag() {
    SPIKE_EXPECT(parse({"spike", "--version"}).command == spike::Command::VERSION);
    SPIKE_EXPECT(parse({"spike", "-v"}).command == spike::Command::VERSION);
}

void test_cli_subcommands_no_args() {
    SPIKE_EXPECT(parse({"spike", "fetch"}).command == spike::Command::FETCH);
    SPIKE_EXPECT(parse({"spike", "dev"}).command   == spike::Command::DEV);
    SPIKE_EXPECT(parse({"spike", "random"}).command == spike::Command::RANDOM);
}

void test_cli_tokens_needs_file() {
    auto missing = parse({"spike", "tokens"});
    SPIKE_EXPECT(missing.command == spike::Command::TOKENS);
    SPIKE_EXPECT(missing.parse_error == true);

    auto ok = parse({"spike", "tokens", "x.por"});
    SPIKE_EXPECT(ok.command == spike::Command::TOKENS);
    SPIKE_EXPECT(ok.parse_error == false);
    SPIKE_EXPECT(ok.input_file == "x.por");
    SPIKE_EXPECT(ok.flag_verbose == false);
}

void test_cli_tokens_accepts_verbose() {
    auto a = parse({"spike", "tokens", "x.por", "--verbose"});
    SPIKE_EXPECT(a.command == spike::Command::TOKENS);
    SPIKE_EXPECT(a.parse_error == false);
    SPIKE_EXPECT(a.flag_verbose);
}

void test_cli_tokens_rejects_unknown_flag() {
    auto a = parse({"spike", "tokens", "x.por", "--xpto"});
    SPIKE_EXPECT(a.parse_error == true);
}

void test_cli_compile_simple() {
    auto a = parse({"spike", "calc.por"});
    SPIKE_EXPECT(a.command == spike::Command::COMPILE);
    SPIKE_EXPECT(a.input_file == "calc.por");
    SPIKE_EXPECT(a.parse_error == false);
}

void test_cli_compile_case_insensitive_extension() {
    auto a = parse({"spike", "CALC.POR"});
    SPIKE_EXPECT(a.command == spike::Command::COMPILE);
    SPIKE_EXPECT(a.input_file == "CALC.POR");
}

void test_cli_compile_with_all_flags() {
    auto a = parse({"spike", "calc.por", "-o", "saida.exe",
                    "--run", "--time", "--verbose", "--c", "--ast"});
    SPIKE_EXPECT(a.command == spike::Command::COMPILE);
    SPIKE_EXPECT(a.input_file == "calc.por");
    SPIKE_EXPECT(a.output_file == "saida.exe");
    SPIKE_EXPECT(a.flag_run);
    SPIKE_EXPECT(a.flag_time);
    SPIKE_EXPECT(a.flag_verbose);
    SPIKE_EXPECT(a.flag_keep_c);
    SPIKE_EXPECT(a.flag_ast);
    SPIKE_EXPECT(a.parse_error == false);
}

void test_cli_compile_flags_in_any_order() {
    auto a = parse({"spike", "calc.por", "--ast", "-o", "out.exe", "--run"});
    SPIKE_EXPECT(a.command == spike::Command::COMPILE);
    SPIKE_EXPECT(a.output_file == "out.exe");
    SPIKE_EXPECT(a.flag_ast);
    SPIKE_EXPECT(a.flag_run);
}

void test_cli_compile_dash_o_missing_value() {
    auto a = parse({"spike", "calc.por", "-o"});
    SPIKE_EXPECT(a.parse_error == true);
}

void test_cli_compile_dash_o_eats_flag_is_error() {
    auto a = parse({"spike", "calc.por", "-o", "--run"});
    SPIKE_EXPECT(a.parse_error == true);
}

void test_cli_compile_unknown_flag_is_error() {
    auto a = parse({"spike", "calc.por", "--xpto"});
    SPIKE_EXPECT(a.parse_error == true);
    SPIKE_EXPECT(a.parse_error_msg.find("--xpto") != std::string::npos);
}

void test_cli_random_modes() {
    SPIKE_EXPECT(parse({"spike", "random"}).random_mode == "");
    SPIKE_EXPECT(parse({"spike", "random", "--basico"}).random_mode  == "basico");
    SPIKE_EXPECT(parse({"spike", "random", "--desafio"}).random_mode == "desafio");
    SPIKE_EXPECT(parse({"spike", "random", "--quiz"}).random_mode    == "quiz");
}

void test_cli_random_unknown_mode_is_error() {
    auto a = parse({"spike", "random", "--xpto"});
    SPIKE_EXPECT(a.parse_error == true);
}

void test_cli_unknown_command_is_error() {
    auto a = parse({"spike", "foobar"});
    SPIKE_EXPECT(a.command == spike::Command::UNKNOWN);
    SPIKE_EXPECT(a.parse_error == true);
}

} // namespace

namespace spike_tests {

void run_cli_tests() {
    test_cli_no_args_is_help();
    test_cli_help_flag();
    test_cli_version_flag();
    test_cli_subcommands_no_args();
    test_cli_tokens_needs_file();
    test_cli_tokens_accepts_verbose();
    test_cli_tokens_rejects_unknown_flag();
    test_cli_compile_simple();
    test_cli_compile_case_insensitive_extension();
    test_cli_compile_with_all_flags();
    test_cli_compile_flags_in_any_order();
    test_cli_compile_dash_o_missing_value();
    test_cli_compile_dash_o_eats_flag_is_error();
    test_cli_compile_unknown_flag_is_error();
    test_cli_random_modes();
    test_cli_random_unknown_mode_is_error();
    test_cli_unknown_command_is_error();
}

} // namespace spike_tests
