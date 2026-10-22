#ifndef SPIKE_BUILTINS_HPP
#define SPIKE_BUILTINS_HPP

#include "spike/ast_enums.hpp"

#include <string>
#include <vector>

namespace spike {

struct BuiltinSig {
    std::string name;
    std::vector<TypeKind> params;
    TypeKind result = TypeKind::None;
    std::string c_name;
    bool takes_line = false;
};

const std::vector<BuiltinSig>& all_builtins();

const BuiltinSig* find_builtin(const std::string& name_lowercase,
                               const std::vector<TypeKind>& arg_types);

bool is_builtin_name(const std::string& name);

std::vector<const BuiltinSig*> builtins_named(const std::string& name);

std::string signature_text(const BuiltinSig& sig);

} // namespace spike

#endif // SPIKE_BUILTINS_HPP
