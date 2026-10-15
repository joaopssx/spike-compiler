#ifndef SPIKE_TEMP_DIR_HPP
#define SPIKE_TEMP_DIR_HPP

#include <filesystem>
#include <string>

namespace spike {

class TempDir {
public:
    TempDir();
    ~TempDir();

    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;

    bool ok() const;
    const std::filesystem::path& path() const;
    const std::string& error() const;

private:
    std::filesystem::path path_;
    std::string error_;
};

bool write_text_file(const std::filesystem::path& path, const std::string& content,
                     std::string& error);

} // namespace spike

#endif // SPIKE_TEMP_DIR_HPP
