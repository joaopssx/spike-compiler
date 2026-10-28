#ifndef SPIKE_SETUP_HPP
#define SPIKE_SETUP_HPP

#include "spike/cli.hpp"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>

namespace spike {

struct ToolchainPin {
    std::string name;
    std::string version;
    std::string url;
    std::uint64_t size = 0;
    std::string sha256;
    std::string archive_name;
    std::string gcc_relative_path;
};

const ToolchainPin& pinned_toolchain();

enum class LinuxFamily { Debian, Fedora, Arch, OpenSuse, Alpine, Unknown };

LinuxFamily detect_linux_family(const std::string& os_release_text);
std::string linux_install_command(LinuxFamily family);

struct SetupPaths {
    std::filesystem::path base;
    std::filesystem::path toolchain_dir;
    std::filesystem::path record_file;
};

SetupPaths setup_paths(const std::filesystem::path& local_app_data, const ToolchainPin& pin);
std::optional<std::filesystem::path> local_app_data_dir();
bool is_inside(const std::filesystem::path& candidate, const std::filesystem::path& base);

// gcc.exe written by a previous "spike setup", if it still exists.
std::optional<std::string> recorded_toolchain_gcc();
std::optional<std::string> recorded_toolchain_gcc(const std::filesystem::path& record_file);

struct SetupError {
    std::string code;
    std::string message;
    std::string hint;
};

using DownloadProgress = std::function<void(std::uint64_t done, std::uint64_t total)>;
// Downloads `url` into `destination`; returns an empty string on success or the reason.
using Downloader = std::function<std::string(const std::string& url,
                                             const std::filesystem::path& destination,
                                             const DownloadProgress& progress)>;
// Extracts `archive` into `folder`; returns an empty string on success or the reason.
using Extractor = std::function<std::string(const std::filesystem::path& archive,
                                            const std::filesystem::path& folder)>;
// Runs "gcc --version"; returns its first line, or nullopt when it does not run.
using CompilerProbe = std::function<std::optional<std::string>(const std::string& gcc)>;

struct InstallTools {
    Downloader download;
    Extractor extract;
    CompilerProbe probe;
    DownloadProgress progress;
};

struct InstallResult {
    bool ok = false;
    std::string gcc_path;
    std::string gcc_version;
    SetupError error;
};

InstallResult install_toolchain(const ToolchainPin& pin, const SetupPaths& paths,
                                const InstallTools& tools);

bool remove_toolchain(const SetupPaths& paths, const std::filesystem::path& target,
                      std::string& error);

InstallTools system_install_tools();

int run_setup(const CliArgs& args);

} // namespace spike

#endif // SPIKE_SETUP_HPP
