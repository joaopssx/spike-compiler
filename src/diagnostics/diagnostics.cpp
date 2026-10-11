#include "spike/diagnostics.hpp"

#include "spike/colors.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace spike {

namespace {

// Divide o conteúdo em linhas usando '\n'. Não inclui '\n' no resultado.
// Faz strip de '\r' final (CRLF) pra mensagens não mostrarem caracteres de
// controle bizarros no console.
std::vector<std::string> split_lines(const std::string& src) {
    std::vector<std::string> out;
    std::string cur;
    cur.reserve(64);
    for (char c : src) {
        if (c == '\n') {
            if (!cur.empty() && cur.back() == '\r') cur.pop_back();
            out.push_back(std::move(cur));
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty() && cur.back() == '\r') cur.pop_back();
    out.push_back(std::move(cur));
    return out;
}

// Largura fixa para o número de linha (à direita). 4 caracteres como o spec pede.
constexpr int kLineNoWidth = 4;

std::string pad_line_number(int n) {
    std::ostringstream oss;
    oss << std::setw(kLineNoWidth) << n;
    return oss.str();
}

std::string blank_line_number() {
    return std::string(kLineNoWidth, ' ');
}

} // namespace

Diagnostics::Diagnostics(const std::string& filename,
                         const std::string& full_source)
    : filename_(filename),
      source_lines_(split_lines(full_source)) {}

void Diagnostics::error(const std::string& code, const std::string& message,
                        int line, int col, const std::string& hint) {
    Diagnostic d;
    d.level = Diagnostic::Level::ERROR;
    d.code = code;
    d.message = message;
    d.filename = filename_;
    d.line = line;
    d.col = col;
    d.source_line = get_line(line);
    d.hint = hint;
    diags_.push_back(std::move(d));
}

void Diagnostics::warning(const std::string& code, const std::string& message,
                          int line, int col, const std::string& hint) {
    Diagnostic d;
    d.level = Diagnostic::Level::WARNING;
    d.code = code;
    d.message = message;
    d.filename = filename_;
    d.line = line;
    d.col = col;
    d.source_line = get_line(line);
    d.hint = hint;
    diags_.push_back(std::move(d));
}

bool Diagnostics::has_errors() const {
    return error_count() > 0;
}

int Diagnostics::error_count() const {
    int n = 0;
    for (const auto& d : diags_) {
        if (d.level == Diagnostic::Level::ERROR) ++n;
    }
    return n;
}

int Diagnostics::warning_count() const {
    int n = 0;
    for (const auto& d : diags_) {
        if (d.level == Diagnostic::Level::WARNING) ++n;
    }
    return n;
}

std::string Diagnostics::get_line(int line_number) const {
    if (line_number < 1) return "";
    const std::size_t idx = static_cast<std::size_t>(line_number - 1);
    if (idx >= source_lines_.size()) return "";
    return source_lines_[idx];
}

std::string Diagnostics::make_caret(int col) const {
    // col é 1-indexed. Caret na coluna 1 → 0 espaços + '^'.
    const int spaces = (col > 0) ? (col - 1) : 0;
    return std::string(static_cast<std::size_t>(spaces), ' ') + "^";
}

void Diagnostics::print_one(const Diagnostic& d) const {
    const bool is_error = d.level == Diagnostic::Level::ERROR;

    // Header: erro[Exxx]: mensagem
    const std::string header_label = is_error
        ? "erro[" + d.code + "]:"
        : "aviso[" + d.code + "]:";
    const std::string header_colored = is_error
        ? red(bold(header_label))
        : yellow(bold(header_label));

    std::cerr << header_colored << " " << d.message << "\n";

    // " --> file:line:col" em ciano
    std::ostringstream loc;
    loc << " --> " << d.filename << ":" << d.line << ":" << d.col;
    std::cerr << cyan(loc.str()) << "\n";

    const std::string bar = gray("|");
    const std::string blank_gutter = blank_line_number() + " " + bar;
    std::cerr << blank_gutter << "\n";

    // Linha anterior (contexto) se existir
    if (d.line > 1) {
        const std::string prev = get_line(d.line - 1);
        std::cerr << gray(pad_line_number(d.line - 1)) << " " << bar << " "
                  << prev << "\n";
    }

    // Linha do erro
    std::cerr << gray(pad_line_number(d.line)) << " " << bar << " "
              << d.source_line << "\n";

    // Linha do caret — sem número de linha
    const std::string caret = make_caret(d.col);
    const std::string caret_colored = is_error ? red(caret) : yellow(caret);
    std::cerr << blank_gutter << " " << caret_colored << "\n";

    // Linha seguinte (contexto) se existir
    if (static_cast<std::size_t>(d.line) < source_lines_.size()) {
        const std::string next = get_line(d.line + 1);
        std::cerr << gray(pad_line_number(d.line + 1)) << " " << bar << " "
                  << next << "\n";
    }

    std::cerr << blank_gutter << "\n";

    // Dica (opcional)
    if (!d.hint.empty()) {
        std::cerr << blank_line_number() << " " << green("= dica:") << " "
                  << d.hint << "\n";
    }

    std::cerr << "\n";
}

void Diagnostics::print_all() const {
    for (const auto& d : diags_) {
        print_one(d);
    }
}

} // namespace spike
