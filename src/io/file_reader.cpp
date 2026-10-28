#include "spike/file_reader.hpp"
#include "diagnostics/utf8.hpp"
#include "io/text_encoding.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>

namespace spike {

namespace {

bool has_por_extension(const std::string& path) {
    std::string ext = std::filesystem::u8path(path).extension().u8string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return ext == ".por";
}

EncodingNote position_of(const std::string& text, std::size_t offset) {
    EncodingNote note;
    note.converted_from_cp1252 = true;
    note.line = 1;
    note.col = 1;
    for (std::size_t i = 0; i < offset && i < text.size(); ++i) {
        if (text[i] == '\n') {
            ++note.line;
            note.col = 1;
        } else {
            ++note.col;
        }
    }
    return note;
}

void prepare_text(ReadResult& result) {
    text_encoding::strip_utf8_bom(result.content);
    std::string text = text_encoding::normalize_newlines(result.content);
    const std::size_t bad = utf8::first_invalid(text);
    if (bad != std::string::npos) {
        if (auto converted = text_encoding::plausible_cp1252_to_utf8(text)) {
            result.encoding = position_of(text, bad);
            text = std::move(*converted);
        }
    }
    result.content = std::move(text);
}

} // namespace

ReadResult read_file(const std::string& path) {
    ReadResult result;

    std::error_code ec;
    const std::filesystem::path file_path = std::filesystem::u8path(path);
    const bool exists = std::filesystem::exists(file_path, ec);
    if (ec || !exists) {
        result.ok = false;
        result.error = "Arquivo não encontrado: " + path;
        return result;
    }

    std::ifstream in(file_path, std::ios::binary);
    if (!in.is_open()) {
        result.ok = false;
        result.error = "Não foi possível abrir: " + path;
        return result;
    }

    result.content.assign(
        std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>()
    );

    prepare_text(result);
    result.ok = true;
    if (!has_por_extension(path)) {
        result.error = "Aviso: o arquivo não tem extensão .por: " + path;
    }
    return result;
}

} // namespace spike
