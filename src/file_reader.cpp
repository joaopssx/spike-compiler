#include "spike/file_reader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace spike {

std::string ReadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("nao foi possivel abrir o arquivo: " + path);
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

}  // namespace spike
