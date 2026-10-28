#ifndef SPIKE_TOOLCHAIN_PINS_HPP
#define SPIKE_TOOLCHAIN_PINS_HPP

#include <cstdint>

// The only file "spike setup" downloads. Every value was checked against the official release
// (github.com/skeeto/w64devkit): see docs/internals.md before changing any of them.
namespace spike::toolchain_pins {

constexpr const char* kName = "w64devkit";
constexpr const char* kVersion = "2.10.1";
constexpr const char* kUrl =
    "https://github.com/skeeto/w64devkit/releases/download/v2.10.1/w64devkit-x64-2.10.1.7z.exe";
constexpr std::uint64_t kSize = 67263552;
constexpr const char* kSha256 = "add1edf984ce4d473cfeb7c52e0ab38db73a855bba6c7f564d5e16648eb2896a";
constexpr const char* kArchiveName = "w64devkit-x64-2.10.1.7z.exe";
constexpr const char* kGccRelativePath = "w64devkit/bin/gcc.exe";

} // namespace spike::toolchain_pins

#endif // SPIKE_TOOLCHAIN_PINS_HPP
