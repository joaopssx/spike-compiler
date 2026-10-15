#ifndef SPIKE_PIPELINE_INTERNAL_HPP
#define SPIKE_PIPELINE_INTERNAL_HPP

#include "spike/ast.hpp"
#include "spike/cc_driver.hpp"
#include "spike/codegen.hpp"
#include "spike/semantic.hpp"

#include <memory>
#include <string>

namespace spike::pipeline_detail {

constexpr int kExitOk = 0;
constexpr int kExitProgramError = 1;
constexpr int kExitInternalError = 3;

struct Analysis {
    std::string path;
    std::string source;
    Program program;
    std::unique_ptr<SemanticAnalyzer> semantic;
};

int analyze_file(const std::string& path, Analysis& analysis);

void print_driver_error(const DriverError& error);
int report_codegen_error(const Analysis& analysis, const CodegenResult& result);
void print_success(const std::string& message);

} // namespace spike::pipeline_detail

#endif // SPIKE_PIPELINE_INTERNAL_HPP
