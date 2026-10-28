#ifndef SPIKE_PIPELINE_INTERNAL_HPP
#define SPIKE_PIPELINE_INTERNAL_HPP

#include "spike/ast.hpp"
#include "spike/cc_driver.hpp"
#include "spike/codegen.hpp"
#include "spike/diagnostics.hpp"
#include "spike/semantic.hpp"
#include "spike/token.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace spike::pipeline_detail {

constexpr int kExitOk = 0;
constexpr int kExitProgramError = 1;
constexpr int kExitInternalError = 3;
constexpr int kTotalSteps = 5;

struct PhaseTime {
    std::string name;
    double ms = 0.0;
};

class Stopwatch {
public:
    double ms() const;

private:
    std::chrono::steady_clock::time_point start_ = std::chrono::steady_clock::now();
};

class Reporter {
public:
    Reporter(bool verbose, bool timing);

    void step(int index, const std::string& text) const;
    void note(const std::string& text) const;
    void add_phase(const std::string& name, double ms);
    void finish() const;
    const std::vector<PhaseTime>& phases() const;
    double total_ms() const;

    void record(const Diagnostics& diagnostics);
    void record_error(const std::string& code);
    std::string with_counts(const std::string& message) const;
    void print_failure() const;

private:
    bool verbose_;
    bool timing_;
    Stopwatch total_;
    std::vector<PhaseTime> phases_;
    int errors_ = 0;
    int warnings_ = 0;
    std::string first_code_;
};

std::string count_text(std::size_t count, const char* singular, const char* plural);

std::string format_time_table(const std::vector<PhaseTime>& phases, double total_ms);

struct Analysis {
    std::string path;
    std::string source;
    Program program;
    std::unique_ptr<SemanticAnalyzer> semantic;
};

struct WarningPolicy {
    bool show = true;
    bool as_errors = false;
};

int read_phase(const std::string& path, Analysis& analysis, Reporter& reporter);
int lex_phase(const Analysis& analysis, std::vector<Token>& tokens, Reporter& reporter);
int parse_phase(Analysis& analysis, std::vector<Token> tokens, Reporter& reporter);
int semantic_phase(Analysis& analysis, Reporter& reporter,
                   const WarningPolicy& warnings = {});
CodegenOptions codegen_options(const Analysis& analysis, bool line_directives);
int codegen_phase(const Analysis& analysis, CodegenResult& generated, Reporter& reporter,
                  bool line_directives = true);

int analyze_file(const std::string& path, Analysis& analysis, Reporter& reporter,
                 bool parse_only = false, const WarningPolicy& warnings = {});

void print_driver_error(const DriverError& error, Reporter& reporter);
int report_codegen_error(const Analysis& analysis, const CodegenResult& result,
                         Reporter& reporter);
void print_success(const std::string& message);

} // namespace spike::pipeline_detail

#endif // SPIKE_PIPELINE_INTERNAL_HPP
