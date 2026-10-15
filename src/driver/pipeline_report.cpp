#include "driver/pipeline_internal.hpp"
#include "spike/colors.hpp"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

namespace spike::pipeline_detail {

namespace {

std::size_t display_width(const std::string& text) {
    std::size_t width = 0;
    for (unsigned char c : text) {
        if ((c & 0xC0) != 0x80) ++width;
    }
    return width;
}

std::string milliseconds(double ms) {
    char buffer[32];
    std::snprintf(buffer, sizeof buffer, "%9.1f ms", ms);
    return buffer;
}

} // namespace

double Stopwatch::ms() const {
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start_)
        .count();
}

Reporter::Reporter(bool verbose, bool timing) : verbose_(verbose), timing_(timing) {}

void Reporter::step(int index, const std::string& text) const {
    if (!verbose_) return;
    std::cerr << gray("[" + std::to_string(index) + "/" + std::to_string(kTotalSteps) + "]")
              << " " << text << std::endl;
}

void Reporter::note(const std::string& text) const {
    if (verbose_) std::cerr << "      " << text << std::endl;
}

void Reporter::add_phase(const std::string& name, double ms) {
    phases_.push_back(PhaseTime{name, ms});
}

void Reporter::finish() const {
    if (timing_) std::cerr << format_time_table(phases_, total_.ms()) << std::flush;
}

std::string count_text(std::size_t count, const char* singular, const char* plural) {
    return std::to_string(count) + " " + (count == 1 ? singular : plural);
}

std::string format_time_table(const std::vector<PhaseTime>& phases, double total_ms) {
    std::size_t width = display_width("total");
    for (const PhaseTime& phase : phases) width = std::max(width, display_width(phase.name));
    auto row = [width](const std::string& name, double ms) {
        return "  " + name + std::string(width - display_width(name), ' ') + milliseconds(ms) +
               "\n";
    };
    std::string out = "tempo por etapa:\n";
    for (const PhaseTime& phase : phases) out += row(phase.name, phase.ms);
    out += row("total", total_ms);
    return out;
}

} // namespace spike::pipeline_detail
