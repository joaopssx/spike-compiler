#ifndef SPIKE_PIPELINE_INTERNAL_HPP
#define SPIKE_PIPELINE_INTERNAL_HPP

#include "spike/ast.hpp"
#include "spike/cc_driver.hpp"
#include "spike/codegen.hpp"
#include "spike/semantic.hpp"

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

private:
    bool verbose_;
    bool timing_;
    Stopwatch total_;
    std::vector<PhaseTime> phases_;
};

std::string count_text(std::size_t count, const char* singular, const char* plural);

std::string format_time_table(const std::vector<PhaseTime>& phases, double total_ms);

struct Analysis {
    std::string path;
    std::string source;
    Program program;
    std::unique_ptr<SemanticAnalyzer> semantic;
};

int analyze_file(const std::string& path, Analysis& analysis, Reporter& reporter,
                 bool parse_only = false);

void print_driver_error(const DriverError& error);
int report_codegen_error(const Analysis& analysis, const CodegenResult& result);
void print_success(const std::string& message);

} // namespace spike::pipeline_detail

#endif // SPIKE_PIPELINE_INTERNAL_HPP
