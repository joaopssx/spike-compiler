#ifndef SPIKE_DIAGNOSTICS_HPP
#define SPIKE_DIAGNOSTICS_HPP

#include <string>
#include <vector>

namespace spike {

struct Diagnostic {
    enum class Level { ERROR, WARNING };

    Level level;
    std::string code;     // ex: "E002"
    std::string message;  // ex: "string não fechada"
    std::string filename;
    int line = 0;
    int col = 0;
    std::string source_line;  // o texto da linha onde ocorreu o erro
    std::string hint;         // dica opcional
};

class Diagnostics {
public:
    explicit Diagnostics(const std::string& filename,
                         const std::string& full_source);

    void error(const std::string& code, const std::string& message,
               int line, int col, const std::string& hint = "");

    void warning(const std::string& code, const std::string& message,
                 int line, int col, const std::string& hint = "");

    void print_all() const;

    bool has_errors() const;
    int error_count() const;
    int warning_count() const;

private:
    std::string filename_;
    std::vector<std::string> source_lines_;
    std::vector<Diagnostic> diags_;

    void print_one(const Diagnostic& d) const;
    std::string get_line(int line_number) const;
    std::string make_caret(int col) const;
};

} // namespace spike

#endif // SPIKE_DIAGNOSTICS_HPP
