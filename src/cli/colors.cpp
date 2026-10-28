#include "spike/colors.hpp"

#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <io.h>
#  include <windows.h>
#else
#  include <unistd.h>
#endif

namespace spike {

namespace {

ColorMode g_mode = ColorMode::Auto;

bool is_set(const char* value) {
    return value != nullptr && value[0] != '\0';
}

bool is_terminal(Stream stream) {
    FILE* file = stream == Stream::Out ? stdout : stderr;
#ifdef _WIN32
    return _isatty(_fileno(file)) != 0;
#else
    return isatty(fileno(file)) != 0;
#endif
}

#ifdef _WIN32
void enable_virtual_terminal(DWORD handle_id) {
    HANDLE h = GetStdHandle(handle_id);
    if (h == INVALID_HANDLE_VALUE) return;
    DWORD mode = 0;
    if (GetConsoleMode(h, &mode)) SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}
#endif

} // namespace

void init_colors() {
#ifdef _WIN32
    enable_virtual_terminal(STD_OUTPUT_HANDLE);
    enable_virtual_terminal(STD_ERROR_HANDLE);
#endif
}

void set_color_mode(ColorMode mode) {
    g_mode = mode;
}

ColorMode color_mode() {
    return g_mode;
}

bool should_color(ColorMode mode, bool terminal, const char* no_color, const char* force_color) {
    if (mode == ColorMode::Always) return true;
    if (mode == ColorMode::Never) return false;
    if (is_set(no_color)) return false;
    if (is_set(force_color) && std::string(force_color) != "0") return true;
    return terminal;
}

bool enabled(Stream stream) {
    return should_color(g_mode, is_terminal(stream), std::getenv("NO_COLOR"),
                        std::getenv("FORCE_COLOR"));
}

std::string paint(const char* code, const std::string& s, Stream to) {
    if (!enabled(to)) return s;
    return std::string(code) + s + Color::RESET;
}

} // namespace spike
