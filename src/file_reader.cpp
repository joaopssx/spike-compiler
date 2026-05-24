#include "spike/file_reader.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace spike {

namespace {

bool has_por_extension(const std::string& path) {
    const std::string ext = ".por";
    if (path.size() < ext.size()) {
        return false;
    }
    // Case-insensitive compare of the trailing 4 chars.
    for (std::size_t i = 0; i < ext.size(); ++i) {
        char a = path[path.size() - ext.size() + i];
        char b = ext[i];
        if (a >= 'A' && a <= 'Z') {
            a = static_cast<char>(a + ('a' - 'A'));
        }
        if (a != b) {
            return false;
        }
    }
    return true;
}

} // namespace

ReadResult read_file(const std::string& path) {
    ReadResult result;

    std::error_code ec;
    const bool exists = std::filesystem::exists(path, ec);
    if (ec || !exists) {
        result.ok = false;
        result.error = "Arquivo não encontrado: " + path;
        return result;
    }

    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        result.ok = false;
        result.error = "Não foi possível abrir: " + path;
        return result;
    }

    result.content.assign(
        std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>()
    );

    result.ok = true;
    if (!has_por_extension(path)) {
        result.error = "Aviso: o arquivo não tem extensão .por: " + path;
    }
    return result;
}

} // namespace spike
