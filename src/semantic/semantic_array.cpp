#include "spike/semantic.hpp"
#include "semantic/semantic_internal.hpp"

#include <string>
#include <variant>

namespace spike {

using semantic_detail::expr_start;
using semantic_detail::quoted;
using semantic_detail::sample_indices;

namespace {

constexpr long long kMaxArrayElements = 10000000;

std::string count_text(std::size_t count, const char* singular, const char* plural) {
    return std::to_string(count) + " " + (count == 1 ? singular : plural);
}

std::string place_text(const Symbol& symbol, std::size_t dimension) {
    if (symbol.dims.size() == 1) return "do vetor '" + symbol.name + "'";
    return "da dimensão " + std::to_string(dimension + 1) + " de '" + symbol.name + "'";
}

} // namespace

namespace semantic_detail {

std::string sample_indices(std::size_t dimensions) {
    if (dimensions >= 3) return "i, j, k";
    return dimensions == 2 ? "i, j" : "i";
}

} // namespace semantic_detail

long long SemanticAnalyzer::bound_value(const ArrayBound& bound, bool& ok) {
    if (bound.constant.empty()) return bound.value;
    const auto it = constants_.find(symbol_key(bound.constant));
    if (it == constants_.end() || it->second.type != TypeKind::Integer) {
        diags_.error("E775", "o limite '" + bound.constant + "' precisa ser uma constante inteira",
                     bound.line, bound.col,
                     "use um número inteiro ou uma constante inteira declarada na seção const.");
        ok = false;
        return 0;
    }
    if (Symbol* symbol = symbols_.lookup(bound.constant)) symbol->used = true;
    return bound.negative ? -it->second.integer : it->second.integer;
}

bool SemanticAnalyzer::resolve_dims(const VarDecl& decl, std::vector<ArrayBounds>& dims) {
    long long total = 1;
    for (const ArrayDim& dim : decl.dims) {
        bool ok = true;
        const long long low = bound_value(dim.low, ok);
        const long long high = bound_value(dim.high, ok);
        if (!ok) return false;
        if (high < low) {
            diags_.error("E775",
                         "o vetor '" + decl.name + "' termina antes de começar (de " +
                             std::to_string(low) + " a " + std::to_string(high) + ")",
                         dim.low.line, dim.low.col, "escreva o menor limite primeiro.");
            return false;
        }
        const long long size = high - low + 1;
        if (size > kMaxArrayElements || total > kMaxArrayElements / size) {
            diags_.error("E775",
                         "o vetor '" + decl.name + "' é grande demais (o limite é " +
                             std::to_string(kMaxArrayElements) + " elementos)",
                         dim.low.line, dim.low.col, "use um vetor menor.");
            return false;
        }
        total *= size;
        dims.push_back(ArrayBounds{low, high});
    }
    return true;
}

TypeKind SemanticAnalyzer::check_index(const IndexExpr& node, bool reading) {
    Symbol* symbol = resolve(node.name, node.line, node.col);
    for (const ExprPtr& index : node.indices) {
        const TypeKind type = check_expr(index);
        if (index && type != TypeKind::Invalid && type != TypeKind::Integer) {
            const SourcePos pos = expr_start(*index);
            diags_.error("E772", "o índice precisa ser inteiro, mas é " + quoted(type), pos.line,
                         pos.col, "use trunca() para transformar um real em inteiro.");
        }
    }
    if (!symbol) return TypeKind::Invalid;
    if (reading) symbol->used = true;
    if (symbol->type == TypeKind::Invalid) return TypeKind::Invalid;
    if (symbol->dims.empty()) {
        diags_.error("E771", "'" + symbol->name + "' não é um vetor", node.line, node.col,
                     "só vetores usam [ ]; declare '" + symbol->name +
                         " : vetor[1..10] de " + type_name(symbol->type) + "'.");
        return TypeKind::Invalid;
    }
    if (node.indices.size() != symbol->dims.size()) {
        diags_.error("E773",
                     "'" + symbol->name + "' tem " +
                         count_text(symbol->dims.size(), "dimensão", "dimensões") +
                         ", mas recebeu " + count_text(node.indices.size(), "índice", "índices"),
                     node.line, node.col,
                     "use um índice para cada dimensão: " + symbol->name + "[" +
                         sample_indices(symbol->dims.size()) + "].");
        return TypeKind::Invalid;
    }
    for (std::size_t i = 0; i < node.indices.size(); ++i) {
        const ExprPtr& index = node.indices[i];
        if (!index || first_non_constant(index) || expr_types_[index.get()] != TypeKind::Integer) {
            continue;
        }
        const ConstEvaluation value = evaluate_constant(*index, constants_);
        const ArrayBounds& bounds = symbol->dims[i];
        if (value.ok && (value.value.integer < bounds.low || value.value.integer > bounds.high)) {
            const SourcePos pos = expr_start(*index);
            diags_.error("E774",
                         "índice " + std::to_string(value.value.integer) + " fora " +
                             place_text(*symbol, i) + " (válido: " + std::to_string(bounds.low) +
                             " a " + std::to_string(bounds.high) + ")",
                         pos.line, pos.col, "use um índice entre os limites da declaração.");
        }
    }
    return symbol->type;
}

Symbol* SemanticAnalyzer::check_target(const ExprPtr& target) {
    if (!target) return nullptr;
    if (const auto* var = std::get_if<VarExpr>(&target->node)) {
        Symbol* symbol = mark_assigned(var->name, var->line, var->col);
        if (symbol && !symbol->dims.empty()) {
            diags_.error("E770", "o vetor '" + symbol->name + "' precisa de um índice", var->line,
                         var->col,
                         "guarde um elemento por vez: " + var->name + "[" +
                             sample_indices(symbol->dims.size()) + "] <- valor.");
            return nullptr;
        }
        return symbol;
    }
    if (const auto* index = std::get_if<IndexExpr>(&target->node)) {
        if (check_index(*index, false) == TypeKind::Invalid) return nullptr;
        Symbol* symbol = symbols_.lookup(index->name);
        if (symbol) symbol->assigned = true;
        return symbol;
    }
    return nullptr;
}

} // namespace spike
