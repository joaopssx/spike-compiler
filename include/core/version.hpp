#pragma once

#include <string_view>

namespace portugol::core {

inline constexpr std::string_view kSpikeVersion = "0.1.0";
inline constexpr std::string_view kSpikeTagline = "A modern Portugol compiler written in C++";

std::string_view versionString() noexcept;
std::string_view tagline() noexcept;

} // namespace portugol::core
