#ifndef SPIKE_COLORS_HPP
#define SPIKE_COLORS_HPP

#include <string>

namespace spike {

enum class ColorMode { Auto, Always, Never };

enum class Stream { Out, Err };

// Turns on ANSI sequences in the Windows console (VT100 mode) for stdout and
// stderr. No-op elsewhere.
void init_colors();

void set_color_mode(ColorMode mode);
ColorMode color_mode();

// Auto: only when the stream is a terminal and NO_COLOR is unset; FORCE_COLOR
// turns colors on even without a terminal. NO_COLOR wins over FORCE_COLOR.
bool should_color(ColorMode mode, bool is_terminal, const char* no_color,
                  const char* force_color);

bool enabled(Stream stream);

namespace Color {
    constexpr const char* RESET   = "\033[0m";
    constexpr const char* BOLD    = "\033[1m";
    constexpr const char* RED     = "\033[31m";
    constexpr const char* GREEN   = "\033[32m";
    constexpr const char* YELLOW  = "\033[33m";
    constexpr const char* CYAN    = "\033[36m";
    constexpr const char* WHITE   = "\033[37m";
    // Caramelo - cor principal do Spike, igual ao pelo do cachorro.
    constexpr const char* CARAMEL = "\033[38;5;130m";
    constexpr const char* GRAY    = "\033[38;5;240m";
} // namespace Color

std::string paint(const char* code, const std::string& s, Stream to);

// The stream argument says where the text will be written: colors depend on it.
inline std::string red(const std::string& s, Stream to = Stream::Err)     { return paint(Color::RED, s, to); }
inline std::string green(const std::string& s, Stream to = Stream::Err)   { return paint(Color::GREEN, s, to); }
inline std::string yellow(const std::string& s, Stream to = Stream::Err)  { return paint(Color::YELLOW, s, to); }
inline std::string cyan(const std::string& s, Stream to = Stream::Err)    { return paint(Color::CYAN, s, to); }
inline std::string bold(const std::string& s, Stream to = Stream::Err)    { return paint(Color::BOLD, s, to); }
inline std::string caramel(const std::string& s, Stream to = Stream::Err) { return paint(Color::CARAMEL, s, to); }
inline std::string gray(const std::string& s, Stream to = Stream::Err)    { return paint(Color::GRAY, s, to); }

} // namespace spike

#endif // SPIKE_COLORS_HPP
