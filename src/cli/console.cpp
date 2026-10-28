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

void restore_output_code_page() {
    if (g_original_output_cp != 0) {
        SetConsoleOutputCP(g_original_output_cp);
    }
}

} // namespace

void init_console() {
    init_colors();
    if (g_original_output_cp != 0) return;
    const UINT current = GetConsoleOutputCP();
    if (current == 0 || current == CP_UTF8) return;
    if (SetConsoleOutputCP(CP_UTF8)) {
        g_original_output_cp = current;
        std::atexit(restore_output_code_page);
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
