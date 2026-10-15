#include "spike/compile_pipeline.hpp"
#include "driver/pipeline_internal.hpp"
#include "spike/cc_driver.hpp"
#include "spike/codegen.hpp"
#include "spike/colors.hpp"
#include "spike/process.hpp"
#include "spike/temp_dir.hpp"

#include <chrono>
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

std::string seconds_since(std::chrono::steady_clock::time_point start) {
    const double seconds =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    char buffer[32];
    std::snprintf(buffer, sizeof buffer, "%.1fs", seconds);
    return buffer;
}

bool clear_old_output(const std::string& output) {
    std::error_code ec;
    const fs::path path = fs::u8path(output);
    if (!fs::exists(path, ec)) return true;
    fs::remove(path, ec);
    if (!ec) return true;
    print_driver_error(write_failed_error(
        output, "o arquivo já existe e não pôde ser substituído (o programa ainda está aberto?)"));
    return false;
}

int run_program(const std::string& output) {
    std::error_code ec;
    const std::string absolute = fs::absolute(fs::u8path(output), ec).u8string();
    std::cout << std::endl;
    ProcessOptions options;
    options.args = {ec ? output : absolute};
    const ProcessResult result = run_process(options);
    if (!result.launched) {
        std::cerr << red(bold("erro:")) << " " << result.launch_error << std::endl;
        return kExitInternalError;
    }
    return result.exit_code;
}

} // namespace

int run_check(const CliArgs& args) {
    Analysis analysis;
    const int status = analyze_file(args.input_file, analysis);
    if (status != kExitOk) return status;
    print_success("nenhum erro encontrado");
    return kExitOk;
}

int run_compile(const CliArgs& args) {
    const auto start = std::chrono::steady_clock::now();
    Analysis analysis;
    const int status = analyze_file(args.input_file, analysis);
    if (status != kExitOk) return status;

    const std::string source_name = fs::u8path(args.input_file).filename().u8string();
    const CodegenResult generated =
        generate_c(analysis.program, analysis.semantic->expr_types(), source_name);
    if (!generated.ok) return report_codegen_error(analysis, generated);

    const std::optional<CompilerInfo> compiler = find_c_compiler();
    if (!compiler) {
        print_driver_error(no_compiler_error());
        return kExitInternalError;
    }

    TempDir temp;
    if (!temp.ok()) {
        print_driver_error(write_failed_error("pasta temporária", temp.error()));
        return kExitInternalError;
    }
    const fs::path c_file = temp.path() / (fs::u8path(source_name).stem().u8string() + ".c");
    std::string write_error;
    if (!write_text_file(c_file, generated.c_source, write_error)) {
        print_driver_error(write_failed_error(c_file.u8string(), write_error));
        return kExitInternalError;
    }

    const std::string output = output_path_for(args.input_file, args.output_file);
    if (!clear_old_output(output)) return kExitInternalError;
    const CompileResult compiled =
        compile_c(*compiler, CompileRequest{c_file.u8string(), output, false});
    if (!compiled.ok) {
        print_driver_error(compiled.error);
        return kExitInternalError;
    }

    if (args.flag_keep_c) {
        const std::string kept = c_file_path_for(output);
        std::error_code ec;
        fs::copy_file(c_file, fs::u8path(kept), fs::copy_options::overwrite_existing, ec);
        if (ec) {
            print_driver_error(write_failed_error(kept, ec.message()));
            return kExitInternalError;
        }
    }

    print_success("gerado '" + output + "' (compilado em " + seconds_since(start) + ")");
    if (args.flag_keep_c) print_success("código C em '" + c_file_path_for(output) + "'");
    if (!args.flag_run) return kExitOk;
    return run_program(output);
}

} // namespace spike
