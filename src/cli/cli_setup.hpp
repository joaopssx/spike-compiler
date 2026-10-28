#ifndef SPIKE_CLI_SETUP_HPP
#define SPIKE_CLI_SETUP_HPP

#include "spike/cli.hpp"

namespace spike {

bool parse_setup_args(int argc, char* argv[], CliArgs& out);

} // namespace spike

#endif // SPIKE_CLI_SETUP_HPP
