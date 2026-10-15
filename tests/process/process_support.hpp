#ifndef SPIKE_TESTS_PROCESS_SUPPORT_HPP
#define SPIKE_TESTS_PROCESS_SUPPORT_HPP

#include "spike/process.hpp"

#include <cstdlib>
#include <optional>
#include <string>
#include <utility>

namespace spike_tests {

inline void set_env(const std::string& name, const std::string& value) {
#ifdef _WIN32
    _putenv_s(name.c_str(), value.c_str());
#else
    setenv(name.c_str(), value.c_str(), 1);
#endif
}

inline void unset_env(const std::string& name) {
#ifdef _WIN32
    _putenv_s(name.c_str(), "");
#else
    unsetenv(name.c_str());
#endif
}

class EnvGuard {
public:
    explicit EnvGuard(std::string name) : name_(std::move(name)), saved_(spike::get_env(name_)) {}
    ~EnvGuard() {
        if (saved_) {
            set_env(name_, *saved_);
        } else {
            unset_env(name_);
        }
    }
    EnvGuard(const EnvGuard&) = delete;
    EnvGuard& operator=(const EnvGuard&) = delete;

private:
    std::string name_;
    std::optional<std::string> saved_;
};

inline std::string without_cr(const std::string& text) {
    std::string out;
    for (char c : text) {
        if (c != '\r') out += c;
    }
    return out;
}

} // namespace spike_tests

#endif // SPIKE_TESTS_PROCESS_SUPPORT_HPP
