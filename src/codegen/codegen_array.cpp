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

void Generator::collect_array_dims() {
    for (const VarDecl& decl : program_.globals) {
        if (decl.dims.empty()) continue;
        std::vector<ArrayBounds> dims;
        for (const ArrayDim& dim : decl.dims) {
            dims.push_back(ArrayBounds{bound_value(dim.low), bound_value(dim.high)});
        }
        var_dims_.emplace(symbol_key(decl.name), std::move(dims));
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
    const ArrayBounds& dim = it->second[0];
    const std::string offset = "spike_index(" + expr(node.indices[0]) + ", " +
                               integer_literal(dim.low) + ", " + integer_literal(dim.high) + ", " +
                               std::to_string(node.line) + ", " + string_literal(node.name) + ")";
    return c_name(node.name) + "[" + offset + "]";
}

std::string Generator::lvalue(const ExprPtr& target, std::string& name) {
    if (const VarExpr* var = target_variable(target)) {
        name = var->name;
        return c_name(var->name);
    }
    if (const auto* index = target ? std::get_if<IndexExpr>(&target->node) : nullptr) {
        name = index->name;
        return element(*index);
    }
    fail("E900", "erro interno do gerador: alvo de atribuição inválido", 0, 0);
    return "";
}

} // namespace spike::codegen_detail
