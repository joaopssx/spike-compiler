#include "spike/compile_pipeline.hpp"

#include <cctype>
#include <string>

namespace spike {

namespace {

bool ends_with_ignoring_case(const std::string& text, const std::string& suffix) {
    if (text.size() < suffix.size()) return false;
    const std::size_t offset = text.size() - suffix.size();
    for (std::size_t i = 0; i < suffix.size(); ++i) {
        const auto a = static_cast<unsigned char>(text[offset + i]);
        const auto b = static_cast<unsigned char>(suffix[i]);
        if (std::tolower(a) != std::tolower(b)) return false;
    }
    return true;
}

bool ends_with_separator(const std::string& text) {
    return !text.empty() && (text.back() == '/' || text.back() == '\\');
}

} // namespace

std::string output_path_for(const std::string& input_file, const std::string& output_flag,
                            bool windows) {
    std::string base = output_flag;
    if (base.empty()) {
        base = input_file;
        if (ends_with_ignoring_case(base, ".por")) base.resize(base.size() - 4);
        if (base.empty() || ends_with_separator(base)) base += "programa";
    }
    if (windows && !ends_with_ignoring_case(base, ".exe")) base += ".exe";
    return base;
}

std::string output_path_for(const std::string& input_file, const std::string& output_flag) {
#ifdef _WIN32
    return output_path_for(input_file, output_flag, true);
#else
    return output_path_for(input_file, output_flag, false);
#endif
}

std::string c_file_path_for(const std::string& output_path) {
    std::string base = output_path;
    if (ends_with_ignoring_case(base, ".exe")) base.resize(base.size() - 4);
    return base + ".c";
}

} // namespace spike
