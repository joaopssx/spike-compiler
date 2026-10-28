#ifndef SPIKE_EXPLAIN_HPP
#define SPIKE_EXPLAIN_HPP

#include <iosfwd>
#include <string>

namespace spike {

// spike --explain [code]: an empty code lists every code. Returns the exit
// code: 0, or 2 for an unknown code (message written to err).
int run_explain(const std::string& code, std::ostream& out, std::ostream& err);

// "E001-E004, E100-E105, ...": consecutive codes collapsed into ranges.
std::string summarize_codes();

} // namespace spike

#endif // SPIKE_EXPLAIN_HPP
