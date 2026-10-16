#ifndef SPIKE_AST_PRINTER_HPP
#define SPIKE_AST_PRINTER_HPP

#include "spike/ast.hpp"

#include <cstddef>
#include <string>

namespace spike {

std::string print_ast(const Program& program);

std::size_t count_statements(const StmtList& stmts);
std::size_t count_statements(const Program& program);

} // namespace spike

#endif // SPIKE_AST_PRINTER_HPP
