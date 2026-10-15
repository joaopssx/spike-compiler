#ifndef SPIKE_SEMANTIC_INTERNAL_HPP
#define SPIKE_SEMANTIC_INTERNAL_HPP

#include "spike/ast.hpp"

#include <string>

namespace spike::semantic_detail {

bool is_numeric(TypeKind type);

std::string quoted(TypeKind type);

SourcePos expr_start(const Expr& expr);

} // namespace spike::semantic_detail

#endif // SPIKE_SEMANTIC_INTERNAL_HPP
