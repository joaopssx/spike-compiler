#ifndef SPIKE_DEV_DUMP_HPP
#define SPIKE_DEV_DUMP_HPP

#include "spike/ast.hpp"
#include "spike/symbol_table.hpp"
#include "spike/token.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace spike {

std::string dump_tokens(const std::vector<Token>& tokens);
std::string dump_symbols(const SymbolTable& symbols);
std::string describe_expr(const Expr& expr);
std::string dump_expr_types(const std::unordered_map<const Expr*, TypeKind>& types);

} // namespace spike

#endif // SPIKE_DEV_DUMP_HPP
