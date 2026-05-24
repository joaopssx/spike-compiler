#ifndef SPIKE_COLORS_HPP
#define SPIKE_COLORS_HPP

#include <string>

#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
   // wingdi.h e cia. despejam macros com nomes triviais que quebram código
   // C++ normal (ex: ERROR vira #define ERROR 0, conflitando com qualquer
   // enum value chamado ERROR). Como colors.hpp é um header interno e fica
   // no topo de tudo, limpamos aqui mesmo.
#  ifdef ERROR
#    undef ERROR
#  endif
#  ifdef OPTIONAL
#    undef OPTIONAL
#  endif
#  ifdef IN
#    undef IN
#  endif
#  ifdef OUT
#    undef OUT
#  endif
#endif

namespace spike {

// Ativa o suporte a sequências ANSI no console do Windows (VT100 mode).
// Em outros sistemas é no-op. Header-only → marcado inline pra evitar
// múltiplas definições quando incluído em mais de uma TU.
inline void init_colors() {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(h, &mode)) {
            SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
#endif
}

namespace Color {
    constexpr const char* RESET   = "\033[0m";
    constexpr const char* BOLD    = "\033[1m";
    constexpr const char* RED     = "\033[31m";
    constexpr const char* GREEN   = "\033[32m";
    constexpr const char* YELLOW  = "\033[33m";
    constexpr const char* CYAN    = "\033[36m";
    constexpr const char* WHITE   = "\033[37m";
    // Caramelo — cor principal do Spike, igual ao pelo do cachorro.
    constexpr const char* CARAMEL = "\033[38;5;130m";
    constexpr const char* GRAY    = "\033[38;5;240m";
} // namespace Color

inline std::string red(const std::string& s)     { return std::string(Color::RED)     + s + Color::RESET; }
inline std::string green(const std::string& s)   { return std::string(Color::GREEN)   + s + Color::RESET; }
inline std::string yellow(const std::string& s)  { return std::string(Color::YELLOW)  + s + Color::RESET; }
inline std::string cyan(const std::string& s)    { return std::string(Color::CYAN)    + s + Color::RESET; }
inline std::string bold(const std::string& s)    { return std::string(Color::BOLD)    + s + Color::RESET; }
inline std::string caramel(const std::string& s) { return std::string(Color::CARAMEL) + s + Color::RESET; }
inline std::string gray(const std::string& s)    { return std::string(Color::GRAY)    + s + Color::RESET; }

} // namespace spike

#endif // SPIKE_COLORS_HPP
