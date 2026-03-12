#pragma once

#include "spike/core/session.hpp"

namespace spike {

struct CommandContext {
    portugol::core::CompilationSession* session {nullptr};
};

} // namespace spike
