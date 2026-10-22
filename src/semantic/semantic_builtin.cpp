#include "spike/builtins.hpp"
#include "spike/semantic.hpp"
#include "spike/symbol_table.hpp"
#include "semantic/semantic_internal.hpp"

#include <string>
#include <vector>

namespace spike {

using semantic_detail::expr_start;
using semantic_detail::quoted;

namespace {

bool accepts(TypeKind wanted, TypeKind value) {
    return wanted == value || (wanted == TypeKind::Real && value == TypeKind::Integer);
}

std::string signatures_hint(const std::vector<const BuiltinSig*>& candidates) {
    std::string text = "use ";
    for (std::size_t i = 0; i < candidates.size(); ++i) {
        if (i > 0) text += " ou ";
        text += signature_text(*candidates[i]);
    }
    return text + ".";
}

std::string accepted_types(const std::vector<const BuiltinSig*>& candidates, std::size_t index) {
    std::vector<TypeKind> types;
    for (const BuiltinSig* sig : candidates) {
        const TypeKind type = sig->params[index];
        bool seen = false;
        for (TypeKind other : types) seen = seen || other == type;
        if (!seen) types.push_back(type);
    }
    std::string text;
    for (std::size_t i = 0; i < types.size(); ++i) {
        if (i > 0) text += " ou ";
        text += quoted(types[i]);
    }
    return text;
}

} // namespace

TypeKind SemanticAnalyzer::check_builtin_call(const std::string& name,
                                              const std::vector<ExprPtr>& args, int line,
                                              int col) {
    std::vector<TypeKind> types;
    for (const ExprPtr& arg : args) types.push_back(check_argument_value(arg));
    const std::vector<const BuiltinSig*> named = builtins_named(name);
    const std::string canonical = named.front()->name;
    const std::string hint = signatures_hint(named);

    std::vector<const BuiltinSig*> candidates;
    for (const BuiltinSig* sig : named) {
        if (sig->params.size() == args.size()) candidates.push_back(sig);
    }
    if (candidates.empty()) {
        const std::size_t expected = named.front()->params.size();
        diags_.error("E781",
                     "'" + canonical + "' espera " + std::to_string(expected) +
                         (expected == 1 ? " argumento" : " argumentos") + ", recebeu " +
                         std::to_string(args.size()),
                     line, col, hint);
        return TypeKind::Invalid;
    }
    for (TypeKind type : types) {
        if (type == TypeKind::Invalid) return TypeKind::Invalid;
    }
    if (const BuiltinSig* sig = find_builtin(symbol_key(canonical), types)) return sig->result;

    std::size_t wrong = 0;
    for (std::size_t i = 0; i < types.size(); ++i) {
        bool ok = false;
        for (const BuiltinSig* sig : candidates) ok = ok || accepts(sig->params[i], types[i]);
        if (!ok) {
            wrong = i;
            break;
        }
    }
    const SourcePos pos = args[wrong] ? expr_start(*args[wrong]) : SourcePos{line, col};
    diags_.error("E782",
                 "o argumento " + std::to_string(wrong + 1) + " de '" + canonical +
                     "' precisa ser " + accepted_types(candidates, wrong) + ", mas é " +
                     quoted(types[wrong]),
                 pos.line, pos.col, hint);
    return TypeKind::Invalid;
}

} // namespace spike
