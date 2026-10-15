#include "spike/process.hpp"
#include "process/utf8_path.hpp"

#include <cstdlib>
#include <filesystem>
#include <system_error>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace spike {

using process_detail::path_from_utf8;
using process_detail::path_to_utf8;

namespace {

#ifdef _WIN32
constexpr char kPathListSeparator = ';';
#else
constexpr char kPathListSeparator = ':';
#endif

bool has_directory_part(const std::string& program) {
#ifdef _WIN32
    return program.find_first_of("/\\") != std::string::npos;
#else
    return program.find('/') != std::string::npos;
#endif
}

std::vector<std::string> candidate_names(const std::string& program) {
#ifdef _WIN32
    if (path_from_utf8(program).extension().empty()) return {program + ".exe", program};
#endif
    return {program};
}

} // namespace

#ifdef _WIN32

namespace process_detail {

std::wstring to_wide(const std::string& text) {
    if (text.empty()) return std::wstring();
    const int size = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                                         nullptr, 0);
    std::wstring out(static_cast<std::size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), out.data(), size);
    return out;
}

std::string from_wide(const std::wstring& text) {
    if (text.empty()) return std::string();
    const int size = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                                         nullptr, 0, nullptr, nullptr);
    std::string out(static_cast<std::size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), out.data(), size,
                        nullptr, nullptr);
    return out;
}

} // namespace process_detail

#endif

std::optional<std::string> get_env(const std::string& name) {
#ifdef _WIN32
    const wchar_t* value = _wgetenv(process_detail::to_wide(name).c_str());
    if (!value) return std::nullopt;
    return process_detail::from_wide(value);
#else
    const char* value = std::getenv(name.c_str());
    if (!value) return std::nullopt;
    return std::string(value);
#endif
}

bool is_executable_file(const std::string& path) {
    std::error_code ec;
    const auto status = std::filesystem::status(path_from_utf8(path), ec);
    if (ec || !std::filesystem::is_regular_file(status)) return false;
#ifdef _WIN32
    return true;
#else
    return access(path.c_str(), X_OK) == 0;
#endif
}

std::optional<std::string> find_executable(const std::string& program) {
    if (program.empty()) return std::nullopt;
    const std::vector<std::string> names = candidate_names(program);
    if (has_directory_part(program)) {
        for (const std::string& name : names) {
            if (is_executable_file(name)) return name;
        }
        return std::nullopt;
    }
    const std::optional<std::string> path_list = get_env("PATH");
    if (!path_list) return std::nullopt;
    std::size_t start = 0;
    while (start <= path_list->size()) {
        std::size_t end = path_list->find(kPathListSeparator, start);
        if (end == std::string::npos) end = path_list->size();
        const std::string dir = path_list->substr(start, end - start);
        start = end + 1;
        if (dir.empty()) continue;
        for (const std::string& name : names) {
            const std::string candidate = path_to_utf8(path_from_utf8(dir) / path_from_utf8(name));
            if (is_executable_file(candidate)) return candidate;
        }
    }
    return std::nullopt;
}

} // namespace spike
