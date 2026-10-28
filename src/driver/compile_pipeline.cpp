#include "spike/compile_pipeline.hpp"
#include "driver/pipeline_internal.hpp"
#include "spike/ast_printer.hpp"
#include "spike/cc_driver.hpp"
#include "spike/codegen.hpp"
#include "spike/colors.hpp"
#include "spike/process.hpp"
#include "spike/temp_dir.hpp"

#include <cstdio>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <system_error>

namespace spike {

using namespace pipeline_detail;

namespace {

namespace fs = std::filesystem;

std::string seconds(double ms) {
    char buffer[32];
    std::snprintf(buffer, sizeof buffer, "%.1fs", ms / 1000.0);
    return buffer;
}

// MinGW's gcc reads its arguments in the ANSI code page, so a name with a Greek letter or a
// user folder such as "C:\\Users\\Joao" with accents would reach it broken. On Windows it
// compiles plain ASCII names inside the temporary folder, and Spike moves the result.
constexpr const char* kWindowsCFile = "programa.c";
constexpr const char* kWindowsExe = "programa.exe";

CompileRequest compile_request(const fs::path& c_file, const std::string& output,
                               const TempDir& temp) {
    CompileRequest request{c_file.u8string(), output, false, std::nullopt};
#ifdef _WIN32
    request.c_file = kWindowsCFile;
    request.output = kWindowsExe;
    request.working_dir = temp.path().u8string();
#else
    (void)temp;
#endif
    return request;
}

bool move_windows_output(const TempDir& temp, const std::string& output, Reporter& reporter) {
#ifdef _WIN32
    std::error_code ec;
    const fs::path built = temp.path() / kWindowsExe;
    const fs::path target = fs::u8path(output);
    fs::rename(built, target, ec);
    if (!ec) return true;
    ec.clear();
    fs::copy_file(built, target, fs::copy_options::overwrite_existing, ec);
    if (!ec) return true;
    print_driver_error(write_failed_error(output, ec.message()), reporter);
    return false;
#else
    (void)temp;
    (void)output;
    (void)reporter;
    return true;
#endif
}

bool prepare_output(const std::string& output, Reporter& reporter) {
    std::error_code ec;
    const fs::path path = fs::u8path(output);
    const fs::path folder = path.parent_path();
    if (!folder.empty() && !fs::exists(folder, ec)) {
        fs::create_directories(folder, ec);
        if (ec) {
            const std::string reason =
                "não foi possível criar a pasta '" + folder.u8string() + "': " + ec.message();
            print_driver_error(write_failed_error(output, reason), reporter);
            return false;
        }
    }
    if (!fs::exists(path, ec)) return true;
    fs::remove(path, ec);
    if (!ec) return true;
    print_driver_error(
        write_failed_error(
            output, "o arquivo já existe e não pôde ser substituído (o programa ainda está aberto?)"),
        reporter);
    return false;
}

int run_program(const std::string& output) {
    std::error_code ec;
    const std::string absolute = fs::absolute(fs::u8path(output), ec).u8string();
    std::cerr << std::endl;
    ProcessOptions options;
    options.args = {ec ? output : absolute};
    const ProcessResult result = run_process(options);
    if (!result.launched) {
        std::cerr << red(bold("erro:")) << " " << result.launch_error << std::endl;
        return kExitInternalError;
    }
    return result.exit_code;
}

int print_tree(const CliArgs& args, Reporter& reporter) {
    Analysis analysis;
    const int status = analyze_file(args.input_file, analysis, reporter, true);
    if (status != kExitOk) return status;
    std::cout << print_ast(analysis.program) << std::flush;
    return kExitOk;
}

int build(const CliArgs& args, const std::string& output, Reporter& reporter) {
    const Stopwatch total;
    Analysis analysis;
    const int status =
        analyze_file(args.input_file, analysis, reporter, false,
                     WarningPolicy{!args.flag_no_warnings, args.flag_warnings_as_errors});
    if (status != kExitOk) return status;

    CodegenResult generated;
    const int generated_status = codegen_phase(analysis, generated, reporter);
    if (generated_status != kExitOk) return generated_status;
    const std::string source_name = fs::u8path(args.input_file).filename().u8string();
    const std::string c_name = codegen_options(analysis, true).c_file_name;

    const std::optional<CompilerInfo> compiler = find_c_compiler();
    if (!compiler) {
        print_driver_error(no_compiler_error(), reporter);
        return kExitInternalError;
    }
    reporter.note("compilador C: " + compiler->kind + " (" + compiler->path + ")");

    TempDir temp;
    if (!temp.ok()) {
        print_driver_error(write_failed_error("pasta temporária", temp.error()), reporter);
        return kExitInternalError;
    }
#ifdef _WIN32
    const fs::path c_file = temp.path() / kWindowsCFile;
#else
    const fs::path c_file = temp.path() / fs::u8path(c_name);
#endif
    std::string write_error;
    if (!write_text_file(c_file, generated.c_source, write_error)) {
        print_driver_error(write_failed_error(c_file.u8string(), write_error), reporter);
        return kExitInternalError;
    }

    if (!prepare_output(output, reporter)) return kExitInternalError;
    const Stopwatch compile_time;
    const CompileResult compiled = compile_c(*compiler, compile_request(c_file, output, temp));
    reporter.add_phase("compilador C", compile_time.ms());
    if (!compiled.ok) {
        DriverError error = compiled.error;
        point_to_source(error, compiled.compiler_output, source_name, analysis.source);
        print_driver_error(error, reporter);
        return kExitInternalError;
    }

    if (!move_windows_output(temp, output, reporter)) return kExitInternalError;

    if (args.flag_keep_c) {
        const std::string kept = c_file_path_for(output);
        const CodegenResult clean =
            generate_c(analysis.program, analysis.semantic->expr_types(),
                       analysis.semantic->constants(), source_name,
                       codegen_options(analysis, false));
        if (!write_text_file(fs::u8path(kept), clean.c_source, write_error)) {
            print_driver_error(write_failed_error(kept, write_error), reporter);
            return kExitInternalError;
        }
    }

    print_success(reporter.with_counts("gerado '" + output + "' (compilado em " +
                                       seconds(total.ms()) + ")"));
    if (args.flag_keep_c) print_success("código C em '" + c_file_path_for(output) + "'");
    return kExitOk;
}

} // namespace

int run_check(const CliArgs& args) {
    Reporter reporter(false, false);
    Analysis analysis;
    const int status = analyze_file(args.input_file, analysis, reporter);
    if (status != kExitOk) {
        reporter.print_failure();
        return status;
    }
    print_success(reporter.with_counts("nenhum erro encontrado"));
    return kExitOk;
}

int run_compile(const CliArgs& args) {
    Reporter reporter(args.flag_verbose, args.flag_time);
    if (args.flag_ast) {
        const int status = print_tree(args, reporter);
        reporter.finish();
        if (status != kExitOk) reporter.print_failure();
        return status;
    }
    const std::string output = output_path_for(args.input_file, args.output_file);
    const int status = build(args, output, reporter);
    reporter.finish();
    if (status != kExitOk) {
        reporter.print_failure();
        return status;
    }
    if (!args.flag_run) return status;
    return run_program(output);
}

} // namespace spike
