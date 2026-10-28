#include "commands/dev_dump.hpp"
#include "driver/pipeline_internal.hpp"
#include "spike/ast_printer.hpp"
#include "spike/commands.hpp"
#include "spike/temp_dir.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace spike {

using namespace pipeline_detail;

namespace {

namespace fs = std::filesystem;

struct DevFile {
    std::string name;
    std::string content;
};

void remove_written(const std::vector<fs::path>& written, const fs::path& folder,
                    bool created_folder) {
    std::error_code ec;
    for (const fs::path& file : written) fs::remove(file, ec);
    if (created_folder) fs::remove(folder, ec);
}

int write_dev_files(const fs::path& folder, const std::vector<DevFile>& files,
                    Reporter& reporter) {
    std::error_code ec;
    const bool created_folder = !fs::exists(folder, ec);
    if (created_folder) fs::create_directories(folder, ec);
    if (ec) {
        print_driver_error(write_failed_error(folder.u8string(), ec.message()), reporter);
        return kExitInternalError;
    }
    std::vector<fs::path> written;
    for (const DevFile& file : files) {
        const fs::path path = folder / fs::u8path(file.name);
        std::string error;
        if (!write_text_file(path, file.content, error)) {
            remove_written(written, folder, created_folder);
            print_driver_error(write_failed_error(path.u8string(), error), reporter);
            return kExitInternalError;
        }
        written.push_back(path);
    }
    return kExitOk;
}

int collect(const std::string& input, std::vector<DevFile>& files, Reporter& reporter) {
    Analysis analysis;
    int status = read_phase(input, analysis, reporter);
    if (status != kExitOk) return status;
    std::vector<Token> tokens;
    status = lex_phase(analysis, tokens, reporter);
    if (status != kExitOk) return status;
    files.push_back(DevFile{"tokens.txt", dump_tokens(tokens)});
    status = parse_phase(analysis, std::move(tokens), reporter);
    if (status != kExitOk) return status;
    files.push_back(DevFile{"ast.txt", print_ast(analysis.program)});
    status = semantic_phase(analysis, reporter);
    if (status != kExitOk) return status;
    files.push_back(DevFile{"symbols.txt", dump_symbols(analysis.semantic->symbols())});
    files.push_back(DevFile{"types.txt", dump_expr_types(analysis.semantic->expr_types())});
    CodegenResult generated;
    status = codegen_phase(analysis, generated, reporter, false);
    if (status != kExitOk) return status;
    const std::string c_name = fs::u8path(input).stem().u8string() + ".c";
    files.push_back(DevFile{c_name, generated.c_source});
    files.push_back(DevFile{"times.txt", format_time_table(reporter.phases(), reporter.total_ms())});
    return kExitOk;
}

} // namespace

std::string dev_folder_for(const std::string& input_file) {
    const fs::path input = fs::u8path(input_file);
    return (input.parent_path() / (input.stem().u8string() + ".dev")).u8string();
}

int run_dev(const CliArgs& args) {
    Reporter reporter(false, false);
    std::vector<DevFile> files;
    int status = collect(args.input_file, files, reporter);
    if (status == kExitOk) {
        status = write_dev_files(fs::u8path(dev_folder_for(args.input_file)), files, reporter);
    }
    if (status != kExitOk) {
        reporter.print_failure();
        return status;
    }
    std::cerr << files.back().content;
    print_success(reporter.with_counts("arquivos do modo dev em '" +
                                       dev_folder_for(args.input_file) + "' (" +
                                       count_text(files.size(), "arquivo", "arquivos") + ")"));
    return kExitOk;
}

} // namespace spike
