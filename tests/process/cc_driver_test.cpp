#include "codegen/codegen_support.hpp"
#include "process/process_support.hpp"
#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/cc_driver.hpp"
#include "spike/process.hpp"
#include "spike/temp_dir.hpp"

#include <filesystem>
#include <string>

namespace {

using spike_tests::EnvGuard;
using spike_tests::set_env;
using spike_tests::unset_env;
using spike_tests::without_cr;

#ifdef _WIN32
constexpr const char* kExeSuffix = ".exe";
#else
constexpr const char* kExeSuffix = "";
#endif

void test_compiler_kind_from_name() {
    SPIKE_EXPECT(spike::compiler_kind("/usr/bin/gcc") == "gcc");
    SPIKE_EXPECT(spike::compiler_kind("C:/msys64/ucrt64/bin/gcc.exe") == "gcc");
    SPIKE_EXPECT(spike::compiler_kind("/usr/bin/clang-17") == "clang");
    SPIKE_EXPECT(spike::compiler_kind("CLANG.EXE") == "clang");
    SPIKE_EXPECT(spike::compiler_kind("/usr/bin/cc") == "other");
}

void test_spike_cc_pointing_nowhere_is_e902() {
    EnvGuard guard("SPIKE_CC");
    set_env("SPIKE_CC", "/caminho/que/nao/existe/gcc");
    SPIKE_EXPECT(!spike::find_c_compiler().has_value());
    const spike::DriverError error = spike::no_compiler_error();
    SPIKE_EXPECT(error.code == "E902");
    SPIKE_EXPECT(error.message.find("SPIKE_CC") != std::string::npos);
    SPIKE_EXPECT(error.message.find("/caminho/que/nao/existe/gcc") != std::string::npos);
}

void test_empty_path_without_spike_cc_is_e902() {
    EnvGuard cc_guard("SPIKE_CC");
    EnvGuard path_guard("PATH");
    unset_env("SPIKE_CC");
    set_env("PATH", (std::filesystem::temp_directory_path() / "spike_sem_compilador").u8string());
    SPIKE_EXPECT(!spike::find_c_compiler().has_value());
    const spike::DriverError error = spike::no_compiler_error();
    SPIKE_EXPECT(error.code == "E902");
    SPIKE_EXPECT(error.message.find("SPIKE_CC") != std::string::npos);
    SPIKE_EXPECT(error.message.find("PATH") != std::string::npos);
    SPIKE_EXPECT(!error.hint.empty());
}

void test_failing_compiler_is_e903() {
    EnvGuard guard("SPIKE_CC");
    set_env("SPIKE_CC", SPIKE_FAKE_CC);
    const auto compiler = spike::find_c_compiler();
    SPIKE_EXPECT(compiler.has_value());
    if (!compiler) return;
    SPIKE_EXPECT(compiler->kind == "other");

    const spike::CompileResult result =
        spike::compile_c(*compiler, spike::CompileRequest{"programa.c", "programa", false});
    SPIKE_EXPECT(!result.ok);
    SPIKE_EXPECT(result.error.code == "E903");
    SPIKE_EXPECT(result.error.message == "o compilador C recusou o código gerado");
    SPIKE_EXPECT(result.error.hint.find("--c") != std::string::npos);
    SPIKE_EXPECT(result.error.hint.find("github.com/joaopssx/spike-compiler") != std::string::npos);
    SPIKE_EXPECT(result.error.details.find("fake error line 20\n") != std::string::npos);
    SPIKE_EXPECT(result.error.details.find("fake error line 21") == std::string::npos);
    SPIKE_EXPECT(result.compiler_output.find("fake error line 25") != std::string::npos);
}

void test_bundled_compiler_is_found_first() {
    EnvGuard guard("SPIKE_CC");
    unset_env("SPIKE_CC");
    spike::TempDir dir;
    SPIKE_EXPECT(dir.ok());
    std::filesystem::create_directories(dir.path() / "bin");
    const auto bundled = dir.path() / "bin" / (std::string("gcc") + kExeSuffix);
    std::filesystem::copy_file(std::filesystem::u8path(SPIKE_FAKE_CC), bundled);
    const auto compiler = spike::find_c_compiler(dir.path().u8string());
    SPIKE_EXPECT(compiler.has_value());
    if (compiler) {
        SPIKE_EXPECT(std::filesystem::equivalent(std::filesystem::u8path(compiler->path), bundled));
        SPIKE_EXPECT(compiler->kind == "gcc");
    }
}

void test_write_failed_error() {
    const spike::DriverError error = spike::write_failed_error("saida/prog.c", "sem permissão");
    SPIKE_EXPECT(error.code == "E904");
    SPIKE_EXPECT(error.message.find("saida/prog.c") != std::string::npos);
    SPIKE_EXPECT(error.hint.find("sem permissão") != std::string::npos);
}

#ifdef SPIKE_TEST_C_COMPILER

void test_real_compiler_found_on_path() {
    EnvGuard cc_guard("SPIKE_CC");
    EnvGuard path_guard("PATH");
    unset_env("SPIKE_CC");
    const auto compiler_dir = std::filesystem::u8path(SPIKE_TEST_C_COMPILER).parent_path();
    set_env("PATH", compiler_dir.u8string());
    const auto compiler = spike::find_c_compiler();
    SPIKE_EXPECT(compiler.has_value());
    if (compiler) {
        SPIKE_EXPECT(std::filesystem::u8path(compiler->path).parent_path() == compiler_dir);
    }
}

void test_real_compile_with_awkward_paths() {
    const spike::CompilerInfo compiler{SPIKE_TEST_C_COMPILER,
                                       spike::compiler_kind(SPIKE_TEST_C_COMPILER)};
    const auto gen = spike_tests::generate_from_source(
        "algoritmo \"t\"\ninicio\n  escreval(\"Olá; $(echo) & fim\")\nfimalgoritmo\n");
    SPIKE_EXPECT(gen.result.ok);

    spike::TempDir dir;
    SPIKE_EXPECT(dir.ok());
    const auto folder = dir.path() / std::filesystem::u8path("pasta com espaço");
    std::filesystem::create_directories(folder);
    const auto c_file = folder / "programa;1.c";
    const auto exe = folder / (std::string("saída & teste") + kExeSuffix);
    std::string error;
    SPIKE_EXPECT(spike::write_text_file(c_file, gen.result.c_source, error));

    const auto result = spike::compile_c(
        compiler, spike::CompileRequest{c_file.u8string(), exe.u8string(), true});
    SPIKE_EXPECT(result.ok);
    if (!result.ok) return;

    spike::ProcessOptions run;
    run.args = {exe.u8string()};
    run.capture_stdout = true;
    const auto ran = spike::run_process(run);
    SPIKE_EXPECT(ran.launched);
    SPIKE_EXPECT(ran.exit_code == 0);
    SPIKE_EXPECT(without_cr(ran.out) == "Olá; $(echo) & fim\n");
}

#else

void test_real_compiler_found_on_path() {}

void test_real_compile_with_awkward_paths() {}

#endif

} // namespace

namespace spike_tests {

void run_cc_driver_tests() {
    test_compiler_kind_from_name();
    test_spike_cc_pointing_nowhere_is_e902();
    test_empty_path_without_spike_cc_is_e902();
    test_failing_compiler_is_e903();
    test_bundled_compiler_is_found_first();
    test_write_failed_error();
    test_real_compiler_found_on_path();
    test_real_compile_with_awkward_paths();
}

} // namespace spike_tests
