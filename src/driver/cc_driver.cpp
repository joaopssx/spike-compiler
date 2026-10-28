#include "spike/cc_driver.hpp"
#include "spike/process.hpp"

#include <cctype>
#include <filesystem>
#include <string>
#include <vector>

namespace spike {

namespace {

constexpr int kMaxCompilerLines = 20;
constexpr const char* kIssuesUrl = "https://github.com/joaopssx/spike-compiler/issues";

std::vector<std::string> path_compiler_names() {
#ifdef _WIN32
    return {"gcc", "clang"};
#else
    return {"cc", "gcc", "clang"};
#endif
}

std::string first_lines(const std::string& text, int max_lines) {
    std::string out;
    int lines = 0;
    std::size_t start = 0;
    while (start < text.size() && lines < max_lines) {
        std::size_t end = text.find('\n', start);
        if (end == std::string::npos) end = text.size();
        std::string line = text.substr(start, end - start);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        out += line + "\n";
        ++lines;
        start = end + 1;
    }
    if (start < text.size()) out += "...\n";
    return out;
}

} // namespace

std::string compiler_kind(const std::string& path) {
    std::string name = std::filesystem::u8path(path).filename().u8string();
    for (char& c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (name.find("clang") != std::string::npos) return "clang";
    if (name.find("gcc") != std::string::npos) return "gcc";
    return "other";
}

std::optional<CompilerInfo> find_c_compiler(const std::string& bundled_dir) {
    const std::optional<std::string> forced = get_env("SPIKE_CC");
    if (forced && !forced->empty()) {
        const std::optional<std::string> path = find_executable(*forced);
        if (!path) return std::nullopt;
        return CompilerInfo{*path, compiler_kind(*path)};
    }
    if (!bundled_dir.empty()) {
        for (const char* name : {"gcc", "clang", "cc"}) {
            const std::string candidate =
                (std::filesystem::u8path(bundled_dir) / "bin" / name).u8string();
            if (const std::optional<std::string> path = find_executable(candidate)) {
                return CompilerInfo{*path, compiler_kind(*path)};
            }
        }
    }
    for (const std::string& name : path_compiler_names()) {
        if (const std::optional<std::string> path = find_executable(name)) {
            return CompilerInfo{*path, compiler_kind(*path)};
        }
    }
    return std::nullopt;
}

CompileResult compile_c(const CompilerInfo& compiler, const CompileRequest& request) {
    ProcessOptions options;
    options.args = {compiler.path, request.optimize ? "-O2" : "-O0", "-std=c99", "-o",
                    request.output, request.c_file, "-lm"};
#ifdef _WIN32
    options.args.push_back("-Wl,--stack,16777216");
#endif
    options.capture_stdout = true;
    options.capture_stderr = true;
    const ProcessResult process = run_process(options);

    CompileResult result;
    result.compiler_output = process.launched ? process.out + process.err : process.launch_error;
    result.ok = process.launched && process.exit_code == 0;
    if (!result.ok) result.error = compiler_failed_error(result.compiler_output);
    return result;
}

DriverError no_compiler_error() {
    DriverError error;
    error.code = "E902";
    const std::optional<std::string> forced = get_env("SPIKE_CC");
    if (forced && !forced->empty()) {
        error.message = "SPIKE_CC aponta para '" + *forced +
                        "', mas esse programa não foi encontrado";
        error.hint = "corrija o caminho em SPIKE_CC ou apague a variável para o Spike procurar "
                     "o compilador sozinho.";
        return error;
    }
    error.message = "nenhum compilador C encontrado (procurei em SPIKE_CC e no PATH)";
#ifdef _WIN32
    error.hint = "instale o MinGW-w64 (por exemplo pelo MSYS2) e coloque a pasta bin dele no "
                 "PATH, ou aponte SPIKE_CC para o gcc.exe.";
#else
    error.hint = "instale o gcc (sudo apt install build-essential) ou o clang.";
#endif
    return error;
}

DriverError compiler_failed_error(const std::string& compiler_output) {
    DriverError error;
    error.code = "E903";
    error.message = "o compilador C recusou o código gerado";
    error.hint = std::string("isso é um defeito do Spike, não do seu programa. Rode com --c para "
                             "salvar o arquivo C e abra uma issue em ") +
                 kIssuesUrl + ".";
    error.details = first_lines(compiler_output, kMaxCompilerLines);
    return error;
}

int compiler_error_line(const std::string& compiler_output, const std::string& source_name) {
    const std::string marker = source_name + ":";
    std::size_t at = compiler_output.find(marker);
    while (at != std::string::npos) {
        const bool name_starts_here =
            at == 0 || std::string("\n/\\ \"'").find(compiler_output[at - 1]) != std::string::npos;
        std::size_t digit = at + marker.size();
        int line = 0;
        while (digit < compiler_output.size() && line < 1000000 &&
               std::isdigit(static_cast<unsigned char>(compiler_output[digit]))) {
            line = line * 10 + (compiler_output[digit] - '0');
            ++digit;
        }
        if (name_starts_here && line > 0) return line;
        at = compiler_output.find(marker, at + 1);
    }
    return 0;
}

void point_to_source(DriverError& error, const std::string& compiler_output,
                     const std::string& source_name, const std::string& source) {
    const int line = compiler_error_line(compiler_output, source_name);
    if (line == 0) return;
    error.message += " para a linha " + std::to_string(line) + " de " + source_name;
    error.location = source_name + ":" + std::to_string(line);
    error.source_line = line;
    std::size_t start = 0;
    for (int i = 1; i < line && start != std::string::npos; ++i) {
        start = source.find('\n', start);
        if (start != std::string::npos) ++start;
    }
    if (start == std::string::npos || start > source.size()) return;
    const std::size_t end = source.find('\n', start);
    std::string text = source.substr(start, end == std::string::npos ? end : end - start);
    if (!text.empty() && text.back() == '\r') text.pop_back();
    error.source_text = text;
}

DriverError write_failed_error(const std::string& path, const std::string& reason) {
    DriverError error;
    error.code = "E904";
    error.message = "não foi possível gravar '" + path + "'";
    error.hint = reason + ". Confira se a pasta existe e se você tem permissão para escrever nela.";
    return error;
}

} // namespace spike
