#include "codegen/codegen_internal.hpp"

#include <string>
#include <variant>

namespace spike::codegen_detail {

long long Generator::bound_value(const ArrayBound& bound) const {
    if (bound.constant.empty()) return bound.value;
    const auto it = constants_.find(symbol_key(bound.constant));
    if (it == constants_.end()) return 0;
    return bound.negative ? -it->second.integer : it->second.integer;
}

std::vector<ArrayBounds> Generator::dims_of(const VarDecl& decl) const {
    std::vector<ArrayBounds> dims;
    for (const ArrayDim& dim : decl.dims) {
        dims.push_back(ArrayBounds{bound_value(dim.low), bound_value(dim.high)});
    }
    return dims;
}

void Generator::collect_array_dims() {
    for (const VarDecl& decl : program_.globals) {
        if (!decl.dims.empty()) var_dims_.emplace(symbol_key(decl.name), dims_of(decl));
    }
}

long long Generator::element_count(const std::string& name) const {
    const auto it = var_dims_.find(symbol_key(name));
    if (it == var_dims_.end()) return 0;
    long long count = 1;
    for (const ArrayBounds& dim : it->second) count *= dim.high - dim.low + 1;
    return count;
}

std::string Generator::element(const IndexExpr& node) {
    const auto it = var_dims_.find(symbol_key(node.name));
    if (it == var_dims_.end() || it->second.size() != node.indices.size()) {
        fail("E900", "erro interno do gerador: '" + node.name + "' não é um vetor", node.line,
             node.col);
        return "";
    }
    const std::vector<ArrayBounds>& dims = it->second;
    const std::string tail = std::to_string(node.line) + ", " + string_literal(node.name);
    std::vector<std::string> positions;
    for (std::size_t i = 0; i < dims.size(); ++i) {
        Access after;
        for (std::size_t j = i + 1; j < dims.size(); ++j) after = after | access(node.indices[j]);
        positions.push_back(expr_as(node.indices[i], TypeKind::Integer, after));
    }
    if (dims.size() == 1) {
        return c_name(node.name) + "[spike_index(" + positions[0] + ", " +
               integer_literal(dims[0].low) + ", " + integer_literal(dims[0].high) + ", " + tail +
               ")]";
    }
    std::string offset;
    for (std::size_t i = 0; i < dims.size(); ++i) {
        const std::string position = "spike_index_dim(" + positions[i] + ", " +
                                     integer_literal(dims[i].low) + ", " +
                                     integer_literal(dims[i].high) + ", " + tail + ", " +
                                     std::to_string(i + 1) + ")";
        if (i == 0) {
            offset = position;
            continue;
        }
        if (i > 1) offset = "(" + offset + ")";
        offset = "(" + offset + " * " + std::to_string(dims[i].high - dims[i].low + 1) + ") + " +
                 position;
    }
    return c_name(node.name) + "[" + offset + "]";
}

std::string Generator::lvalue(const ExprPtr& target, std::string& name) {
    if (const VarExpr* var = target_variable(target)) {
        name = var->name;
        return variable(var->name);
    }
    if (const auto* index = target ? std::get_if<IndexExpr>(&target->node) : nullptr) {
        name = index->name;
        return element(*index);
    }
    fail("E900", "erro interno do gerador: alvo de atribuição inválido", 0, 0);
    return "";
}

} // namespace spike::codegen_detail
