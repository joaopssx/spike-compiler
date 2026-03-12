#pragma once

#include <iosfwd>

#include "core/session.hpp"

namespace portugol::cli {

void printUsage(std::ostream& out);
core::SessionOptions parseOptions(int argc, char** argv);

} // namespace portugol::cli
