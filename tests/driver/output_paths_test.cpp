#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/cli.hpp"
#include "spike/compile_pipeline.hpp"

#include <string>
#include <vector>

namespace {

using spike::c_file_path_for;
using spike::output_path_for;

spike::CliArgs parse(const std::vector<std::string>& argv_in) {
    std::vector<char*> argv;
    for (const auto& s : argv_in) argv.push_back(const_cast<char*>(s.c_str()));
    return spike::parse_args(static_cast<int>(argv.size()), argv.data());
}

void test_default_output_strips_por() {
    SPIKE_EXPECT(output_path_for("hello.por", "", false) == "hello");
    SPIKE_EXPECT(output_path_for("hello.por", "", true) == "hello.exe");
    SPIKE_EXPECT(output_path_for("HELLO.POR", "", false) == "HELLO");
    SPIKE_EXPECT(output_path_for("Media.Por", "", true) == "Media.exe");
}

void test_default_output_keeps_folder() {
    SPIKE_EXPECT(output_path_for("pasta/sub/prog.por", "", false) == "pasta/sub/prog");
    SPIKE_EXPECT(output_path_for("../aula 3/meu prog.por", "", false) == "../aula 3/meu prog");
    SPIKE_EXPECT(output_path_for("C:\\Users\\João\\meu prog.por", "", true) ==
                 "C:\\Users\\João\\meu prog.exe");
    SPIKE_EXPECT(output_path_for("pasta/.por", "", false) == "pasta/programa");
}

void test_output_flag_wins() {
    SPIKE_EXPECT(output_path_for("x.por", "saida/bin", false) == "saida/bin");
    SPIKE_EXPECT(output_path_for("x.por", "saida/bin", true) == "saida/bin.exe");
    SPIKE_EXPECT(output_path_for("x.por", "a.EXE", true) == "a.EXE");
    SPIKE_EXPECT(output_path_for("x.por", "com espaço", true) == "com espaço.exe");
    SPIKE_EXPECT(output_path_for("x.por", "prog.exe", false) == "prog.exe");
}

void test_c_file_next_to_executable() {
    SPIKE_EXPECT(c_file_path_for("prog.exe") == "prog.c");
    SPIKE_EXPECT(c_file_path_for("pasta/prog") == "pasta/prog.c");
    SPIKE_EXPECT(c_file_path_for("PROG.EXE") == "PROG.c");
}

void test_cli_check_command() {
    const auto ok = parse({"spike", "check", "a.por"});
    SPIKE_EXPECT(ok.command == spike::Command::CHECK);
    SPIKE_EXPECT(!ok.parse_error);
    SPIKE_EXPECT(ok.input_file == "a.por");
    SPIKE_EXPECT(parse({"spike", "check"}).parse_error);
    SPIKE_EXPECT(parse({"spike", "check", "a.por", "--run"}).parse_error);
}

} // namespace

namespace spike_tests {

void run_output_path_tests() {
    test_default_output_strips_por();
    test_default_output_keeps_folder();
    test_output_flag_wins();
    test_c_file_next_to_executable();
    test_cli_check_command();
}

} // namespace spike_tests
