#pragma once

#include <chrono>
#include <string>
#include <vector>

namespace portugol::core {

struct PhaseRecord {
    std::string name;
    std::chrono::nanoseconds duration {};
};

class PhaseProfiler {
public:
    void record(std::string phaseName, std::chrono::nanoseconds duration);

    const std::vector<PhaseRecord>& phases() const noexcept;
    std::chrono::nanoseconds totalDuration() const noexcept;
    std::string report() const;

    static std::string formatDuration(std::chrono::nanoseconds duration);

private:
    std::vector<PhaseRecord> phases_;
};

} // namespace portugol::core
