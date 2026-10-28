#ifndef SPIKE_WARNINGS_HPP
#define SPIKE_WARNINGS_HPP

#include "spike/ast.hpp"
#include "spike/diagnostics.hpp"
#include "spike/semantic.hpp"

namespace spike {

void check_warnings(const Program& program, const SemanticAnalyzer& semantic, Diagnostics& out);

} // namespace spike

#endif // SPIKE_WARNINGS_HPP
