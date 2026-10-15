#ifndef SPIKE_COMPILE_PIPELINE_HPP
#define SPIKE_COMPILE_PIPELINE_HPP

#include "spike/cli.hpp"

#include <string>

namespace spike {

std::string output_path_for(const std::string& input_file, const std::string& output_flag,
                            bool windows);

std::string output_path_for(const std::string& input_file, const std::string& output_flag);

std::string c_file_path_for(const std::string& output_path);

int run_compile(const CliArgs& args);

int run_check(const CliArgs& args);

} // namespace spike

#endif // SPIKE_COMPILE_PIPELINE_HPP
