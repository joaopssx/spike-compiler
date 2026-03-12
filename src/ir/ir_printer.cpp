#include "ir.hpp"

#include <sstream>

namespace portugol {

namespace {

std::string jsonEscape(std::string_view text) {
    std::string escaped;
    for (const char ch : text) {
        switch (ch) {
        case '"':
            escaped += "\\\"";
            break;
        case '\\':
            escaped += "\\\\";
            break;
        default:
            escaped.push_back(ch);
            break;
        }
    }
    return escaped;
}

} // namespace

std::string dumpIr(const IRProgram& program) {
    std::ostringstream out;
    out << "IR Program \"" << program.name << "\"\n\n";
    for (const auto& instruction : program.instructions) {
        out << instruction.opcode;
        if (!instruction.operands.empty()) {
            out << " ";
            for (std::size_t index = 0; index < instruction.operands.size(); ++index) {
                if (index > 0) {
                    out << ", ";
                }
                out << instruction.operands[index];
            }
        }
        out << "\n";
    }
    return out.str();
}

std::string irToJson(const IRProgram& program) {
    std::ostringstream out;
    out << "{\n";
    out << "  \"name\": \"" << jsonEscape(program.name) << "\",\n";
    out << "  \"instructions\": [\n";
    for (std::size_t index = 0; index < program.instructions.size(); ++index) {
        const auto& instruction = program.instructions[index];
        out << "    {\n";
        out << "      \"opcode\": \"" << jsonEscape(instruction.opcode) << "\",\n";
        out << "      \"operands\": [";
        for (std::size_t operandIndex = 0; operandIndex < instruction.operands.size(); ++operandIndex) {
            if (operandIndex > 0) {
                out << ", ";
            }
            out << "\"" << jsonEscape(instruction.operands[operandIndex]) << "\"";
        }
        out << "]\n";
        out << "    }";
        if (index + 1 < program.instructions.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

} // namespace portugol
