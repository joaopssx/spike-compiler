#ifndef SPIKE_UTF8_PATH_HPP
#define SPIKE_UTF8_PATH_HPP

#include <filesystem>
#include <string>

namespace spike::process_detail {

inline std::filesystem::path path_from_utf8(const std::string& text) {
    return std::filesystem::u8path(text);
}

inline std::string path_to_utf8(const std::filesystem::path& path) {
    return path.u8string();
}

#ifdef _WIN32
std::wstring to_wide(const std::string& text);
std::string from_wide(const std::wstring& text);
#endif

} // namespace spike::process_detail

#endif // SPIKE_UTF8_PATH_HPP
