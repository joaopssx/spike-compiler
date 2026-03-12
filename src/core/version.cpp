#include "core/version.hpp"

namespace portugol::core {

std::string_view versionString() noexcept {
    return kSpikeVersion;
}

std::string_view tagline() noexcept {
    return kSpikeTagline;
}

} // namespace portugol::core
