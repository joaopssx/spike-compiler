#include "spike/console.hpp"
#include "spike/colors.hpp"

#include <cstdlib>

#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#endif

namespace spike {

#ifdef _WIN32

namespace {

UINT g_original_output_cp = 0;
UINT g_original_input_cp = 0;
bool g_restore_registered = false;

void restore_code_pages() {
    if (g_original_output_cp != 0) SetConsoleOutputCP(g_original_output_cp);
    if (g_original_input_cp != 0) SetConsoleCP(g_original_input_cp);
    g_original_output_cp = 0;
    g_original_input_cp = 0;
}

} // namespace

void init_console() {
    init_colors();
    const UINT output = GetConsoleOutputCP();
    if (g_original_output_cp == 0 && output != 0 && output != CP_UTF8 &&
        SetConsoleOutputCP(CP_UTF8)) {
        g_original_output_cp = output;
    }
    const UINT input = GetConsoleCP();
    if (g_original_input_cp == 0 && input != 0 && input != CP_UTF8 && SetConsoleCP(CP_UTF8)) {
        g_original_input_cp = input;
    }
    if (!g_restore_registered && (g_original_output_cp != 0 || g_original_input_cp != 0)) {
        g_restore_registered = true;
        std::atexit(restore_code_pages);
    }
}

bool console_uses_utf8() {
    const UINT current = GetConsoleOutputCP();
    return current == 0 || current == CP_UTF8;
}

#else

void init_console() {
    init_colors();
}

bool console_uses_utf8() {
    return true;
}

#endif

} // namespace spike
