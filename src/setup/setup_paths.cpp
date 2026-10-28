#include "spike/process.hpp"
#include "spike/setup.hpp"
#include "setup/toolchain_pins.hpp"

#include <filesystem>
#include <fstream>
#include <string>

namespace spike {

namespace fs = std::filesystem;

const ToolchainPin& pinned_toolchain() {
    static const ToolchainPin pin{toolchain_pins::kName,        toolchain_pins::kVersion,
                                  toolchain_pins::kUrl,         toolchain_pins::kSize,
                                  toolchain_pins::kSha256,      toolchain_pins::kArchiveName,
                                  toolchain_pins::kGccRelativePath};
    return pin;
}

SetupPaths setup_paths(const fs::path& local_app_data, const ToolchainPin& pin) {
    SetupPaths paths;
    paths.base = local_app_data / "Spike";
    paths.toolchain_dir = paths.base / "toolchain" / fs::u8path(pin.version);
    paths.record_file = paths.base / "toolchain.txt";
    return paths;
}

std::optional<fs::path> local_app_data_dir() {
#ifdef _WIN32
    const std::optional<std::string> value = get_env("LOCALAPPDATA");
    if (value && !value->empty()) return fs::u8path(*value);
#endif
    return std::nullopt;
}

bool is_inside(const fs::path& candidate, const fs::path& base) {
    std::error_code ec;
    const fs::path inner = fs::weakly_canonical(candidate, ec);
    if (ec) return false;
    const fs::path outer = fs::weakly_canonical(base, ec);
    if (ec) return false;
    const fs::path relative = inner.lexically_relative(outer);
    if (relative.empty() || relative == ".") return false;
    return *relative.begin() != "..";
}

std::optional<std::string> recorded_toolchain_gcc(const fs::path& record_file) {
    std::ifstream in(record_file, std::ios::binary);
    std::string line;
    if (!in || !std::getline(in, line)) return std::nullopt;
    while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) line.pop_back();
    std::error_code ec;
    if (line.empty() || !fs::is_regular_file(fs::u8path(line), ec)) return std::nullopt;
    return line;
}

std::optional<std::string> recorded_toolchain_gcc() {
    const std::optional<fs::path> local = local_app_data_dir();
    if (!local) return std::nullopt;
    return recorded_toolchain_gcc(setup_paths(*local, pinned_toolchain()).record_file);
}

} // namespace spike
