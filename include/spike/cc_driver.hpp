#ifndef SPIKE_CC_DRIVER_HPP
#define SPIKE_CC_DRIVER_HPP

#include <optional>
#include <string>

namespace spike {

struct CompilerInfo {
    std::string path;
    std::string kind;
};

struct DriverError {
    std::string code;
    std::string message;
    std::string hint;
    std::string details;
};

struct CompileRequest {
    std::string c_file;
    std::string output;
    bool optimize = false;
};

struct CompileResult {
    bool ok = false;
    std::string compiler_output;
    DriverError error;
};

std::string compiler_kind(const std::string& path);

std::optional<CompilerInfo> find_c_compiler(const std::string& bundled_dir = "");

CompileResult compile_c(const CompilerInfo& compiler, const CompileRequest& request);

DriverError no_compiler_error();
DriverError compiler_failed_error(const std::string& compiler_output);
DriverError write_failed_error(const std::string& path, const std::string& reason);

} // namespace spike

#endif // SPIKE_CC_DRIVER_HPP
