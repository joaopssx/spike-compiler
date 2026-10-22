#include "spike/builtins.hpp"
#include "spike/symbol_table.hpp"

#include <string>
#include <vector>

namespace spike {

namespace {

constexpr TypeKind kInt = TypeKind::Integer;
constexpr TypeKind kReal = TypeKind::Real;

bool convertible(const BuiltinSig& sig, const std::vector<TypeKind>& args) {
    if (sig.params.size() != args.size()) return false;
    for (std::size_t i = 0; i < args.size(); ++i) {
        const bool widened = sig.params[i] == kReal && args[i] == kInt;
        if (sig.params[i] != args[i] && !widened) return false;
    }
    return true;
}

} // namespace

const std::vector<BuiltinSig>& all_builtins() {
    static const std::vector<BuiltinSig> kBuiltins = {
        {"abs", {kInt}, kInt, "spike_abs_int", false},
        {"abs", {kReal}, kReal, "fabs", false},
        {"raiz", {kReal}, kReal, "spike_sqrt", true},
        {"pot", {kReal, kReal}, kReal, "spike_pow", true},
        {"arredonda", {kReal}, kInt, "spike_round", true},
        {"trunca", {kReal}, kInt, "spike_trunc", true},
        {"piso", {kReal}, kInt, "spike_floor", true},
        {"teto", {kReal}, kInt, "spike_ceil", true},
        {"seno", {kReal}, kReal, "sin", false},
        {"cosseno", {kReal}, kReal, "cos", false},
        {"tangente", {kReal}, kReal, "tan", false},
        {"log", {kReal}, kReal, "spike_log", true},
        {"exp", {kReal}, kReal, "exp", false},
        {"aleatorio", {kInt, kInt}, kInt, "spike_random", true},
    };
    return kBuiltins;
}

const BuiltinSig* find_builtin(const std::string& name_lowercase,
                               const std::vector<TypeKind>& arg_types) {
    const BuiltinSig* widened = nullptr;
    for (const BuiltinSig& sig : all_builtins()) {
        if (sig.name != name_lowercase) continue;
        if (sig.params == arg_types) return &sig;
        if (!widened && convertible(sig, arg_types)) widened = &sig;
    }
    return widened;
}

bool is_builtin_name(const std::string& name) {
    return !builtins_named(name).empty();
}

std::vector<const BuiltinSig*> builtins_named(const std::string& name) {
    const std::string key = symbol_key(name);
    std::vector<const BuiltinSig*> found;
    for (const BuiltinSig& sig : all_builtins()) {
        if (sig.name == key) found.push_back(&sig);
    }
    return found;
}

std::string signature_text(const BuiltinSig& sig) {
    std::string text = sig.name + "(";
    for (std::size_t i = 0; i < sig.params.size(); ++i) {
        if (i > 0) text += ", ";
        text += type_name(sig.params[i]);
    }
    return text + "): " + type_name(sig.result);
}

} // namespace spike
