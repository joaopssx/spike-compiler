#include "support/registry.hpp"
#include "support/test_support.hpp"

#include "spike/file_reader.hpp"

#include <filesystem>
#include <fstream>
#include <string>

namespace {

// ---- file_reader -----------------------------------------------------------

void test_read_file_missing() {
    spike::ReadResult r = spike::read_file("definitely_does_not_exist_xyz.por");
    SPIKE_EXPECT(r.ok == false);
    SPIKE_EXPECT(r.error.find("Arquivo não encontrado") != std::string::npos);
}

void test_read_file_ok() {
    const std::string path = "test_temp.por";
    const std::string payload = "algoritmo \"teste\"\n";
    {
        std::ofstream out(path, std::ios::binary);
        out << payload;
    }
    spike::ReadResult r = spike::read_file(path);
    SPIKE_EXPECT(r.ok == true);
    SPIKE_EXPECT(r.content == payload);
    SPIKE_EXPECT(r.error.empty());
    std::error_code ec;
    std::filesystem::remove(path, ec);
}

void test_read_file_wrong_extension_still_ok() {
    const std::string path = "test_temp.txt";
    const std::string payload = "qualquer coisa";
    {
        std::ofstream out(path, std::ios::binary);
        out << payload;
    }
    spike::ReadResult r = spike::read_file(path);
    SPIKE_EXPECT(r.ok == true);
    SPIKE_EXPECT(r.content == payload);
    SPIKE_EXPECT(!r.error.empty());
    std::error_code ec;
    std::filesystem::remove(path, ec);
}

} // namespace

namespace spike_tests {

void run_file_reader_tests() {
    test_read_file_missing();
    test_read_file_ok();
    test_read_file_wrong_extension_still_ok();
}

} // namespace spike_tests
