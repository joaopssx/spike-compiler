#include "process/process_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/process.hpp"
#include "spike/temp_dir.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace {

using spike_tests::without_cr;

spike::ProcessResult run_echo(const std::vector<std::string>& extra, bool capture = true) {
    spike::ProcessOptions options;
    options.args = {SPIKE_ECHO_ARGS};
    options.args.insert(options.args.end(), extra.begin(), extra.end());
    options.capture_stdout = capture;
    options.capture_stderr = capture;
    return spike::run_process(options);
}

void test_process_captures_output() {
    const auto r = run_echo({"olá", "mundo"});
    SPIKE_EXPECT(r.launched);
    SPIKE_EXPECT(r.exit_code == 0);
    SPIKE_EXPECT(without_cr(r.out) == "olá\nmundo\n");
    SPIKE_EXPECT(r.err.empty());
}

void test_process_exit_code_is_propagated() {
    const auto r = run_echo({"--exit", "7"});
    SPIKE_EXPECT(r.launched);
    SPIKE_EXPECT(r.exit_code == 7);
    SPIKE_EXPECT(run_echo({"--exit", "3"}, false).exit_code == 3);
}

void test_process_missing_program_is_not_launched() {
    spike::ProcessOptions options;
    options.args = {"spike_programa_que_nao_existe_123"};
    const auto r = spike::run_process(options);
    SPIKE_EXPECT(!r.launched);
    SPIKE_EXPECT(r.launch_error.find("não encontrado") != std::string::npos);

    options.args = {(std::filesystem::temp_directory_path() / "nao_existe" / "prog").u8string()};
    SPIKE_EXPECT(!spike::run_process(options).launched);
    SPIKE_EXPECT(!spike::run_process(spike::ProcessOptions{}).launched);
}

void test_process_arguments_arrive_intact() {
    const std::vector<std::string> tricky = {
        "com espaço",  "\"aspas\"",     "a;b",        "$(whoami)",     "`id`",
        "x && y",      "x | more",      "%PATH%",     "barra\\",       "duas\\\\",
        "\\\"misto\\", "",              "ação ñ",     "* ? [abc]",     "> saida.txt",
    };
    const auto r = run_echo(tricky);
    SPIKE_EXPECT(r.launched);
    std::string expected;
    for (const std::string& arg : tricky) expected += arg + "\n";
    SPIKE_EXPECT(without_cr(r.out) == expected);
}

void test_process_stdin_file() {
    spike::TempDir dir;
    SPIKE_EXPECT(dir.ok());
    const auto input = dir.path() / "entrada.txt";
    std::string error;
    SPIKE_EXPECT(spike::write_text_file(input, "linha 1\nlinha 2\n", error));

    spike::ProcessOptions options;
    options.args = {SPIKE_ECHO_ARGS, "--cat"};
    options.stdin_file = input.u8string();
    options.capture_stdout = true;
    const auto r = spike::run_process(options);
    SPIKE_EXPECT(r.launched);
    SPIKE_EXPECT(r.out == "linha 1\nlinha 2\n");

    options.stdin_file = (dir.path() / "nao_existe.txt").u8string();
    const auto missing = spike::run_process(options);
    SPIKE_EXPECT(!missing.launched);
    SPIKE_EXPECT(!missing.launch_error.empty());
}

void test_process_large_output_does_not_block() {
    const auto r = run_echo({"--big"});
    SPIKE_EXPECT(r.launched);
    SPIKE_EXPECT(r.out.size() == 1024u * 1024u);
    SPIKE_EXPECT(r.err.size() == 1024u * 1024u);
}

void test_find_executable() {
    SPIKE_EXPECT(spike::find_executable(SPIKE_ECHO_ARGS).has_value());
    SPIKE_EXPECT(!spike::find_executable("spike_programa_que_nao_existe_123").has_value());
    SPIKE_EXPECT(!spike::find_executable("").has_value());
#ifdef _WIN32
    std::string without_extension = SPIKE_ECHO_ARGS;
    without_extension.resize(without_extension.size() - 4);
    SPIKE_EXPECT(spike::find_executable(without_extension).has_value());
#endif
}

void test_temp_dir_is_removed() {
    std::filesystem::path kept;
    {
        spike::TempDir dir;
        SPIKE_EXPECT(dir.ok());
        kept = dir.path();
        SPIKE_EXPECT(std::filesystem::is_directory(kept));
        std::filesystem::create_directory(kept / "sub");
        std::string error;
        SPIKE_EXPECT(spike::write_text_file(kept / "sub" / "a.c", "int x;", error));
    }
    SPIKE_EXPECT(!std::filesystem::exists(kept));
    spike::TempDir a;
    spike::TempDir b;
    SPIKE_EXPECT(a.path() != b.path());
}

} // namespace

namespace spike_tests {

void run_process_tests() {
    test_process_captures_output();
    test_process_exit_code_is_propagated();
    test_process_missing_program_is_not_launched();
    test_process_arguments_arrive_intact();
    test_process_stdin_file();
    test_process_large_output_does_not_block();
    test_find_executable();
    test_temp_dir_is_removed();
}

} // namespace spike_tests
