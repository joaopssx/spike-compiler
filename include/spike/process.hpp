#ifndef SPIKE_PROCESS_HPP
#define SPIKE_PROCESS_HPP

#include <optional>
#include <string>
#include <vector>

namespace spike {

struct ProcessOptions {
    std::vector<std::string> args;
    std::optional<std::string> stdin_file;
    bool capture_stdout = false;
    bool capture_stderr = false;
};

struct ProcessResult {
    bool launched = false;
    int exit_code = -1;
    std::string out;
    std::string err;
    std::string launch_error;
};

ProcessResult run_process(const ProcessOptions& options);

std::optional<std::string> find_executable(const std::string& program);

bool is_executable_file(const std::string& path);

std::optional<std::string> get_env(const std::string& name);

} // namespace spike

#endif // SPIKE_PROCESS_HPP
