#include "spike/file_reader.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace spike {

namespace {

bool has_por_extension(const std::string& path) {
    std::string ext = std::filesystem::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return ext == ".por";
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
