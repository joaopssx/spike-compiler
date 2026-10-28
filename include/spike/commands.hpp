#ifndef SPIKE_COMMANDS_HPP
#define SPIKE_COMMANDS_HPP

#include "spike/cli.hpp"

#include <ostream>
#include <string>

namespace spike {

std::string os_description();
std::string arch_name();
std::string first_line(const std::string& text);

int run_fetch(std::ostream& out);

std::string dev_folder_for(const std::string& input_file);

int run_dev(const CliArgs& args);

int run_random(std::ostream& err);

} // namespace spike

#endif // SPIKE_COMMANDS_HPP
