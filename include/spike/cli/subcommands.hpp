#pragma once

#include <string_view>

namespace spike {

inline constexpr std::string_view kBuildCommand = "build";
inline constexpr std::string_view kRunCommand = "run";
inline constexpr std::string_view kCheckCommand = "check";
inline constexpr std::string_view kTokensCommand = "tokens";
inline constexpr std::string_view kAstCommand = "ast";
inline constexpr std::string_view kEmitCCommand = "emit-c";
inline constexpr std::string_view kTraceCommand = "trace";
inline constexpr std::string_view kDoctorCommand = "doctor";
inline constexpr std::string_view kVersionCommand = "version";

} // namespace spike
