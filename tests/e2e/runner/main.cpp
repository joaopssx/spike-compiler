#include "e2e_runner.hpp"

#include "spike/process.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <system_error>

namespace {

namespace fs = std::filesystem;
using namespace spike_e2e;

struct Case {
    fs::path dir;
    fs::path work;
    std::string spike;
    std::string name;
};

int expected_code(const Case& c, int fallback) {
    const auto text = read_text(c.dir / "expected_code.txt");
    return text ? std::atoi(text->c_str()) : fallback;
}

spike::ProcessResult compile(const Case& c, const fs::path& exe) {
    spike::ProcessOptions options;
    options.args = {c.spike, (c.dir / "program.por").u8string(), "-o", exe.u8string()};
    options.capture_stdout = true;
    options.capture_stderr = true;
    return spike::run_process(options);
}

bool no_c_compiler(const spike::ProcessResult& result) {
    return result.exit_code == 3 && strip_ansi(result.err).find("E902") != std::string::npos;
}

int fail(const Case& c, const std::string& report) {
    std::cout << "FAIL " << c.name << "\n" << report;
    return kFailed;
}

int run_compile_error_case(const Case& c) {
    const spike::ProcessResult result = compile(c, c.work / "program");
    if (!result.launched) return fail(c, "  could not start spike: " + result.launch_error + "\n");
    const std::string err = normalize(strip_ansi(result.err));
    std::string report;
    const int want_code = expected_code(c, 1);
    if (result.exit_code != want_code) {
        report += "  spike exit code " + std::to_string(result.exit_code) + ", expected " +
                  std::to_string(want_code) + "\n";
    }
    const auto expected = read_text(c.dir / "expected_diag.txt");
    if (const auto missing = missing_lines_in_order(normalize(*expected), err)) {
        report += "  diagnostic line not found (in order): \"" + *missing + "\"\n";
    }
    if (!report.empty()) return fail(c, report + "  stderr was:\n" + err);
    std::cout << "PASS " << c.name << "\n";
    return kPassed;
}

int run_program_case(const Case& c) {
    fs::path exe = c.work / "program";
#ifdef _WIN32
    exe += ".exe";
#endif
    const spike::ProcessResult built = compile(c, exe);
    if (!built.launched) return fail(c, "  could not start spike: " + built.launch_error + "\n");
    if (no_c_compiler(built)) {
        std::cout << "SKIP " << c.name << ": no C compiler\n";
        return kSkipped;
    }
    if (built.exit_code != 0) {
        return fail(c, "  spike exited with " + std::to_string(built.exit_code) + "\n" +
                           normalize(strip_ansi(built.out + built.err)));
    }

    fs::path input = c.dir / "input.txt";
    if (!fs::exists(input)) {
        input = c.work / "empty_input.txt";
        std::ofstream(input, std::ios::binary).flush();
    }
    spike::ProcessOptions options;
    options.args = {exe.u8string()};
    options.stdin_file = input.u8string();
    options.capture_stdout = true;
    options.capture_stderr = true;
    const auto start = std::chrono::steady_clock::now();
    const spike::ProcessResult ran = spike::run_process(options);
    const double seconds =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    if (!ran.launched) return fail(c, "  could not start the program: " + ran.launch_error + "\n");

    const std::string out = normalize(ran.out);
    const std::string err = normalize(ran.err);
    std::string report;
    const int want_code = expected_code(c, 0);
    if (ran.exit_code != want_code) {
        report += "  exit code " + std::to_string(ran.exit_code) + ", expected " +
                  std::to_string(want_code) + "\n";
    }
    const auto want_out = read_text(c.dir / "expected_out.txt");
    if (!want_out) return fail(c, "  missing expected_out.txt\n");
    report += first_difference(normalize(*want_out), out);
    if (const auto want_err = read_text(c.dir / "expected_err.txt")) {
        if (const auto missing = missing_lines_in_order(normalize(*want_err), err)) {
            report += "  stderr line not found (in order): \"" + *missing + "\"\n";
        }
    }
    if (const auto limit = read_text(c.dir / "time_limit_seconds.txt")) {
        const double max_seconds = std::atof(limit->c_str());
        if (seconds > max_seconds) {
            report += "  took " + std::to_string(seconds) + "s, limit is " + *limit + "\n";
        }
    }
    if (!report.empty()) {
        return fail(c, report + "  stdout was:\n" + out + "  stderr was:\n" + err);
    }
    std::cout << "PASS " << c.name << "\n";
    return kPassed;
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "usage: spike_e2e <spike executable> <case folder> <work folder>\n";
        return 2;
    }
    Case c{fs::u8path(argv[2]), fs::u8path(argv[3]), argv[1], fs::u8path(argv[2]).filename().u8string()};
    std::error_code ec;
    fs::create_directories(c.work, ec);
    if (ec) return fail(c, "  could not create " + c.work.u8string() + ": " + ec.message() + "\n");
    if (!fs::exists(c.dir / "program.por")) return fail(c, "  missing program.por\n");
    if (fs::exists(c.dir / "expected_diag.txt")) return run_compile_error_case(c);
    return run_program_case(c);
}
