#include "core/phase_profiler.hpp"

#include <iomanip>
#include <sstream>
#include <utility>

namespace portugol::core {

void PhaseProfiler::record(std::string phaseName, std::chrono::nanoseconds duration) {
    phases_.push_back(PhaseRecord {std::move(phaseName), duration});
}

const std::vector<PhaseRecord>& PhaseProfiler::phases() const noexcept {
    return phases_;
}

std::chrono::nanoseconds PhaseProfiler::totalDuration() const noexcept {
    std::chrono::nanoseconds total {0};
    for (const auto& phase : phases_) {
        total += phase.duration;
    }
    return total;
}

std::string PhaseProfiler::report() const {
    std::ostringstream out;
    out << "Spike Timing Report\n\n";
    for (const auto& phase : phases_) {
        out << phase.name << ": " << formatDuration(phase.duration) << "\n";
    }
    out << "\nTotal: " << formatDuration(totalDuration()) << "\n";
    return out.str();
}

std::string PhaseProfiler::formatDuration(std::chrono::nanoseconds duration) {
    const double milliseconds = static_cast<double>(duration.count()) / 1000000.0;
    std::ostringstream out;
    out << std::fixed << std::setprecision(3) << milliseconds << " ms";
    return out.str();
}

} // namespace portugol::core
