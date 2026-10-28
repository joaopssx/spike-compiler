#ifndef SPIKE_FILE_READER_HPP
#define SPIKE_FILE_READER_HPP

#include <string>

namespace spike {

struct EncodingNote {
    bool converted_from_cp1252 = false;
    int line = 0;
    int col = 0;
};

struct ReadResult {
    bool ok = false;
    std::string content;
    std::string error;
    EncodingNote encoding;
};

// Reads the entire contents of `path` (UTF-8, also on Windows) into memory.
//
// On success: ok = true and content is the text every phase sees: without a UTF-8 BOM,
// with "\r\n" and lone "\r" turned into "\n", and converted to UTF-8 when the file was
// plausible Windows-1252 (then encoding says where the first converted byte was). A file
// that is neither UTF-8 nor plausible Windows-1252 is returned as is, for the lexer's E005.
// error = "" or a warning when the file does not have the `.por` extension.
// On failure: ok = false, content = "", error = human-readable message
// in Portuguese describing what went wrong.
ReadResult read_file(const std::string& path);

} // namespace spike

#endif // SPIKE_FILE_READER_HPP
