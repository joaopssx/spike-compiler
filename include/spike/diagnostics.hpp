#ifndef SPIKE_DIAGNOSTICS_HPP
#define SPIKE_DIAGNOSTICS_HPP

#include <string>
#include <vector>

namespace spike {

struct Diagnostic {
    enum class Level { ERROR, WARNING };

    Level level;
    std::string code;     // ex: "E002"
    std::string message;  // ex: "string nao fechada"
    std::string filename;
    int line = 0;
    int col = 0;
    std::string source_line;  // o texto da linha onde ocorreu o erro
    std::string hint;         // dica opcional
};

class Diagnostics {
public:
    // Errors after this many are only counted, then summarized by print_all().
    static constexpr int kMaxErrors = 20;

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

    // Code of the error with the earliest position, or "" when there is none.
    std::string first_error_code() const;

private:
    std::string filename_;
    std::vector<std::string> source_lines_;
    std::vector<Diagnostic> diags_;
    int stored_errors_ = 0;
    int omitted_errors_ = 0;

    void print_one(const Diagnostic& d) const;
    std::string get_line(int line_number) const;
    std::string make_caret(int col) const;
};

} // namespace spike

#endif // SPIKE_DIAGNOSTICS_HPP
