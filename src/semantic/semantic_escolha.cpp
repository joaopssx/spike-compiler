#include "spike/semantic.hpp"
#include "semantic/semantic_internal.hpp"

#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace spike {

using semantic_detail::expr_start;
using semantic_detail::quoted;

namespace {

struct IntRange {
    long long low = 0;
    long long high = 0;
};

bool selector_type_allowed(TypeKind type) {
    return type == TypeKind::Integer || type == TypeKind::String || type == TypeKind::Invalid;
}

std::string repeated_message(long long low, long long high) {
    if (low == high) return "o valor " + std::to_string(low) + " aparece em dois casos";
    return "os valores de " + std::to_string(low) + " a " + std::to_string(high) +
           " aparecem em dois casos";
}

} // namespace

TypeKind SemanticAnalyzer::check_case_value(const ExprPtr& value, TypeKind selector) {
    if (!value) return TypeKind::Invalid;
    if (const VarExpr* bad = first_non_constant(value)) {
        if (symbols_.lookup(bad->name)) {
            diags_.error("E763", "o valor de um caso precisa ser fixo (literal ou constante)",
                         bad->line, bad->col,
                         "'" + bad->name + "' é uma variável; use um número, um texto ou uma "
                                           "constante.");
        } else {
            resolve(bad->name, bad->line, bad->col);
        }
        return TypeKind::Invalid;
    }
    const TypeKind type = check_expr(value);
    if (type == TypeKind::Invalid || selector == TypeKind::Invalid) return TypeKind::Invalid;
    if (type != selector) {
        const SourcePos pos = expr_start(*value);
        diags_.error("E761",
                     "o caso tem tipo " + quoted(type) + ", mas o 'escolha' é de " +
                         quoted(selector),
                     pos.line, pos.col, "use valores do mesmo tipo do valor escolhido.");
        return TypeKind::Invalid;
    }
    return type;
}

void SemanticAnalyzer::check_escolha(const EscolhaStmt& stmt) {
    TypeKind selector = check_expr(stmt.selector);
    if (!selector_type_allowed(selector)) {
        const SourcePos pos =
            stmt.selector ? expr_start(*stmt.selector) : SourcePos{stmt.line, stmt.col};
        diags_.error("E760", "'escolha' aceita inteiro ou caractere, mas o valor é " +
                                 quoted(selector),
                     pos.line, pos.col,
                     selector == TypeKind::Real
                         ? "para um real, use 'se' com comparações, ou trunca() para um inteiro."
                         : "para um logico, use 'se ... senao'.");
        selector = TypeKind::Invalid;
    }

    std::vector<IntRange> seen_numbers;
    std::set<std::string> seen_texts;
    for (const CaseClause& clause : stmt.cases) {
        for (const CaseLabel& label : clause.labels) {
            const TypeKind first = check_case_value(label.value, selector);
            TypeKind last = first;
            if (label.range_end) {
                last = check_case_value(label.range_end, selector);
                if (first == TypeKind::String || last == TypeKind::String) {
                    diags_.error("E765", "intervalo com 'ate' só funciona com inteiros",
                                 label.line, label.col,
                                 "para textos, liste cada valor: caso \"a\", \"b\".");
                    continue;
                }
            }
            if (first == TypeKind::Invalid || last == TypeKind::Invalid) continue;

            const ConstEvaluation low = evaluate_constant(*label.value, constants_);
            const ConstEvaluation high =
                label.range_end ? evaluate_constant(*label.range_end, constants_) : low;
            if (!low.ok || !high.ok) continue;

            if (first == TypeKind::String) {
                if (!seen_texts.insert(low.value.text).second) {
                    diags_.error("E762",
                                 "o valor \"" + low.value.text + "\" aparece em dois casos",
                                 label.line, label.col, "cada valor só pode aparecer uma vez.");
                }
                continue;
            }
            const IntRange range{low.value.integer, high.value.integer};
            if (range.low > range.high) {
                diags_.error("E764",
                             "o intervalo começa em " + std::to_string(range.low) +
                                 " e termina em " + std::to_string(range.high),
                             label.line, label.col,
                             "escreva o menor valor primeiro: caso " + std::to_string(range.high) +
                                 " ate " + std::to_string(range.low) + ".");
                continue;
            }
            for (const IntRange& other : seen_numbers) {
                if (range.low <= other.high && other.low <= range.high) {
                    const long long from = std::max(range.low, other.low);
                    const long long to = std::min(range.high, other.high);
                    diags_.error("E762", repeated_message(from, to),
                                 label.line, label.col, "cada valor só pode aparecer uma vez.");
                    break;
                }
            }
            seen_numbers.push_back(range);
        }
        check_block(clause.body);
    }
    check_block(stmt.default_body);
}

} // namespace spike
