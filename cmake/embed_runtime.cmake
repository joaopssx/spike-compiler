if(NOT DEFINED INPUTS OR NOT DEFINED OUTPUT)
    message(FATAL_ERROR "usage: cmake -DINPUTS=<a.h|b.h> -DOUTPUT=<file> -P embed_runtime.cmake")
endif()

string(REPLACE "|" ";" input_files "${INPUTS}")
set(hex "")
foreach(input IN LISTS input_files)
    file(READ "${input}" part HEX)
    string(APPEND hex "${part}")
endforeach()
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," bytes "${hex}")
string(REPLACE "0x0d,0x0a," "0x0a," bytes "${bytes}")
string(REPEAT "0x[0-9a-f][0-9a-f]," 16 sixteen_bytes)
string(REGEX REPLACE "(${sixteen_bytes})" "\\1\n    " bytes "${bytes}")

file(WRITE "${OUTPUT}"
"#include \"spike/runtime_text.hpp\"

namespace spike {

namespace {

const unsigned char kRuntimeSource[] = {
    ${bytes}0x00
};

} // namespace

const char* runtime_source() {
    return reinterpret_cast<const char*>(kRuntimeSource);
}

std::size_t runtime_source_size() {
    return sizeof(kRuntimeSource) - 1;
}

} // namespace spike
")
