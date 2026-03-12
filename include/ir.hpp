#pragma once

#include <string>
#include <vector>

namespace portugol {

struct IRInstruction {
    std::string opcode;
    std::vector<std::string> operands;
};

struct IRProgram {
    std::string name;
    std::vector<IRInstruction> instructions;
};

std::string dumpIr(const IRProgram& program);
std::string irToJson(const IRProgram& program);

} // namespace portugol
