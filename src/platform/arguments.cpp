#include "spike/platform.hpp"

#include <string>
#include <vector>

#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#  include <shellapi.h>
#endif

namespace spike {

namespace {

std::vector<std::string> copy_arguments(int argc, char** argv) {
    std::vector<std::string> arguments;
    for (int i = 0; i < argc; ++i) arguments.emplace_back(argv[i] ? argv[i] : "");
    return arguments;
}

#ifdef _WIN32

std::string to_utf8(const wchar_t* text) {
    const int size = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 1) return std::string();
    std::string out(static_cast<std::size_t>(size - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text, -1, out.data(), size, nullptr, nullptr);
    return out;
}

#endif

} // namespace

#ifdef _WIN32

std::vector<std::string> utf8_arguments(int argc, char** argv) {
    int count = 0;
    LPWSTR* wide = CommandLineToArgvW(GetCommandLineW(), &count);
    if (wide == nullptr || count != argc) {
        if (wide != nullptr) LocalFree(wide);
        return copy_arguments(argc, argv);
    }
    std::vector<std::string> arguments;
    for (int i = 0; i < count; ++i) arguments.push_back(to_utf8(wide[i]));
    LocalFree(wide);
    return arguments;
}

#else

std::vector<std::string> utf8_arguments(int argc, char** argv) {
    return copy_arguments(argc, argv);
}

#endif

} // namespace spike
