#include "spike/temp_dir.hpp"

#include <fstream>
#include <random>
#include <sstream>
#include <system_error>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace spike {

namespace {

unsigned long current_process_id() {
#ifdef _WIN32
    return static_cast<unsigned long>(GetCurrentProcessId());
#else
    return static_cast<unsigned long>(getpid());
#endif
}

} // namespace

TempDir::TempDir() {
    std::error_code ec;
    const std::filesystem::path base = std::filesystem::temp_directory_path(ec);
    if (ec) {
        error_ = "não foi possível achar a pasta temporária do sistema: " + ec.message();
        return;
    }
    std::random_device device;
    std::mt19937_64 random(device());
    for (int attempt = 0; attempt < 16; ++attempt) {
        std::ostringstream name;
        name << "spike-" << current_process_id() << '-' << std::hex << random();
        const std::filesystem::path candidate = base / name.str();
        if (std::filesystem::create_directory(candidate, ec)) {
            path_ = candidate;
            return;
        }
        if (ec) break;
    }
    error_ = "não foi possível criar uma pasta temporária em " + base.u8string() +
             (ec ? ": " + ec.message() : "");
}

TempDir::~TempDir() {
    if (path_.empty()) return;
    std::error_code ec;
    std::filesystem::remove_all(path_, ec);
}

bool TempDir::ok() const {
    return !path_.empty();
}

const std::filesystem::path& TempDir::path() const {
    return path_;
}

const std::string& TempDir::error() const {
    return error_;
}

bool write_text_file(const std::filesystem::path& path, const std::string& content,
                     std::string& error) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        error = "não foi possível abrir o arquivo para escrita";
        return false;
    }
    file << content;
    file.close();
    if (!file) {
        error = "não foi possível gravar o arquivo inteiro";
        return false;
    }
    return true;
}

} // namespace spike
