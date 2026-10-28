#ifndef SPIKE_WARNINGS_INTERNAL_HPP
#define SPIKE_WARNINGS_INTERNAL_HPP

#include "spike/ast.hpp"
#include "spike/diagnostics.hpp"

namespace spike::warnings_detail {

void check_flow_warnings(const Program& program, Diagnostics& out);

} // namespace spike::warnings_detail

#endif // SPIKE_WARNINGS_INTERNAL_HPP
