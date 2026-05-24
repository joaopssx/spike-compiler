#ifndef SPIKE_FILE_READER_HPP
#define SPIKE_FILE_READER_HPP

#include <string>

namespace spike {

struct ReadResult {
    bool ok = false;
    std::string content;
    std::string error;
};

// Reads the entire contents of `path` into memory.
//
// On success: ok = true, content = file bytes, error = "" (or a warning
// when the file does not have the `.por` extension).
// On failure: ok = false, content = "", error = human-readable message
// in Portuguese describing what went wrong.
ReadResult read_file(const std::string& path);

} // namespace spike

#endif // SPIKE_FILE_READER_HPP
