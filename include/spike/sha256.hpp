#ifndef SPIKE_SHA256_HPP
#define SPIKE_SHA256_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

namespace spike {

class Sha256 {
public:
    Sha256();
    void update(const void* data, std::size_t size);
    std::array<std::uint8_t, 32> finish();
    std::string finish_hex();

private:
    std::array<std::uint32_t, 8> state_;
    std::array<std::uint8_t, 64> block_{};
    std::size_t block_size_ = 0;
    std::uint64_t total_bytes_ = 0;

    void compress(const std::uint8_t* block);
};

std::string sha256_hex(const std::string& data);

// Hash of a file read in pieces; nullopt when the file cannot be read. `path` is UTF-8.
std::optional<std::string> sha256_file(const std::string& path);

} // namespace spike

#endif // SPIKE_SHA256_HPP
