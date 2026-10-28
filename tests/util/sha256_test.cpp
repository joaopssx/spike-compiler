#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/sha256.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

void test_nist_vectors() {
    SPIKE_EXPECT(spike::sha256_hex("") ==
                 "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    SPIKE_EXPECT(spike::sha256_hex("abc") ==
                 "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    SPIKE_EXPECT(spike::sha256_hex("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") ==
                 "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
    SPIKE_EXPECT(spike::sha256_hex(std::string(1000000, 'a')) ==
                 "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
}

void test_padding_boundaries() {
    SPIKE_EXPECT(spike::sha256_hex(std::string(55, 'a')) ==
                 "9f4390f8d30c2dd92ec9f095b65e2b9ae9b0a925a5258e241c9f1e910f734318");
    SPIKE_EXPECT(spike::sha256_hex(std::string(56, 'a')) ==
                 "b35439a4ac6f0948b6d6f9e3c6af0f5f590ce20f1bde7090ef7970686ec6738a");
    SPIKE_EXPECT(spike::sha256_hex(std::string(63, 'a')) ==
                 "7d3e74a05d7db15bce4ad9ec0658ea98e3f06eeecf16b4c6fff2da457ddc2f34");
    SPIKE_EXPECT(spike::sha256_hex(std::string(64, 'a')) ==
                 "ffe054fe7ae0cb6dc65c3af9b61d5209f439851db43d0ba5997337df154668eb");
    SPIKE_EXPECT(spike::sha256_hex(std::string(65, 'a')) ==
                 "635361c48bb9eab14198e76ea8ab7f1a41685d6ad62aa9146d301d4f17eb0ae0");
}

void test_pieces_match_one_shot() {
    const std::string text(1000000, 'a');
    spike::Sha256 hash;
    std::size_t offset = 0;
    for (std::size_t piece = 1; offset < text.size(); piece = piece * 3 % 997 + 1) {
        const std::size_t size = std::min(piece, text.size() - offset);
        hash.update(text.data() + offset, size);
        offset += size;
    }
    SPIKE_EXPECT(hash.finish_hex() ==
                 "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
}

void test_large_file() {
    const std::filesystem::path path =
        std::filesystem::u8path(std::string(SPIKE_BINARY_DIR) + "/sha256_million_a.bin");
    {
        std::ofstream out(path, std::ios::binary);
        out << std::string(1000000, 'a');
    }
    const auto hash = spike::sha256_file(path.u8string());
    SPIKE_EXPECT(hash && *hash == "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
    SPIKE_EXPECT(!spike::sha256_file(path.u8string() + ".nao-existe"));
}

} // namespace

namespace spike_tests {

void run_sha256_tests() {
    test_nist_vectors();
    test_padding_boundaries();
    test_pieces_match_one_shot();
    test_large_file();
}

} // namespace spike_tests
