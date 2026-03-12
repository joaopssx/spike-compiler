#pragma once

#include "spike/cli/command_result.hpp"

namespace spike {

class CommandDispatcher {
public:
    static CommandResult dispatch();
};

} // namespace spike
