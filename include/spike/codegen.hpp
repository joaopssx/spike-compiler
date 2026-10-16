#ifndef SPIKE_CODEGEN_HPP
#define SPIKE_CODEGEN_HPP

#include "spike/ast.hpp"
#include "spike/const_value.hpp"

#include <string>
#include <unordered_map>

namespace spike {

struct CodegenResult {
    bool ok = false;
    std::string c_source;
    std::string error_code;
    std::string error_message;
    int error_line = 0;
    int error_col = 0;
};

CodegenResult generate_c(const Program& program,
                         const std::unordered_map<const Expr*, TypeKind>& expr_types,
                         const ConstantMap& constants,
                         const std::string& source_name = "programa.por");

} // namespace spike

#endif // SPIKE_CODEGEN_HPP
