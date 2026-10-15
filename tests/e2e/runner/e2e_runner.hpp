#ifndef SPIKE_E2E_RUNNER_HPP
#define SPIKE_E2E_RUNNER_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace spike_e2e {

constexpr int kPassed = 0;
constexpr int kFailed = 1;
constexpr int kSkipped = 77;

std::optional<std::string> read_text(const std::filesystem::path& path);
std::string normalize(const std::string& text);
std::string strip_ansi(const std::string& text);
std::vector<std::string> split_lines(const std::string& text);

std::string first_difference(const std::string& expected, const std::string& actual);
std::optional<std::string> missing_lines_in_order(const std::string& expected,
                                                  const std::string& actual);

} // namespace spike_e2e

#endif // SPIKE_E2E_RUNNER_HPP
