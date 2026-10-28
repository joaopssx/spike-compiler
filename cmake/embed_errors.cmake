cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED INPUT OR NOT DEFINED OUTPUT)
    message(FATAL_ERROR "usage: cmake -DINPUT=docs/errors.md -DOUTPUT=<file> -P embed_errors.cmake")
endif()

# Characters that CMake lists treat specially are swapped for markers while
# the file is split into lines, and swapped back when the C++ is written.
file(READ "${INPUT}" text)
string(REPLACE "\r\n" "\n" text "${text}")
string(REPLACE "\\" "<<SPIKE_BS>>" text "${text}")
string(REPLACE ";" "<<SPIKE_SC>>" text "${text}")
string(REPLACE "[" "<<SPIKE_LB>>" text "${text}")
string(REPLACE "]" "<<SPIKE_RB>>" text "${text}")
string(REPLACE "\n" ";" lines "${text}")

set(fields message when fix example)
set(codes "")
set(code "")
set(field "")
set(in_code_block FALSE)

macro(finish_entry)
    if(NOT code STREQUAL "")
        foreach(name IN LISTS fields)
            string(STRIP "${entry_${code}_${name}}" entry_${code}_${name})
            if(entry_${code}_${name} STREQUAL "")
                message(FATAL_ERROR "docs/errors.md: ${code} has no '${name}' part "
                    "(needs **Mensagem:**, **Quando acontece:**, **Como corrigir:** and a code block)")
            endif()
        endforeach()
    endif()
    set(code "")
    set(field "")
endmacro()

foreach(line IN LISTS lines)
    if(in_code_block)
        if(line MATCHES "^```")
            set(in_code_block FALSE)
            set(field "")
        elseif(field STREQUAL "example")
            string(APPEND entry_${code}_example "${line}\n")
        endif()
    elseif(line MATCHES "^## ([EWR][0-9][0-9][0-9]) — (.+)$")
        finish_entry()
        set(code "${CMAKE_MATCH_1}")
        if(code IN_LIST codes)
            message(FATAL_ERROR "docs/errors.md: ${code} appears twice")
        endif()
        list(APPEND codes "${code}")
        set(entry_${code}_title "${CMAKE_MATCH_2}")
        foreach(name IN LISTS fields)
            set(entry_${code}_${name} "")
        endforeach()
    elseif(line MATCHES "^##? " OR line STREQUAL "---")
        if(line MATCHES "^## [EWR][0-9]")
            message(FATAL_ERROR "docs/errors.md: bad title '${line}' (expected '## E400 — title')")
        endif()
        finish_entry()
    elseif(code STREQUAL "")
    elseif(line MATCHES "^```")
        set(in_code_block TRUE)
        if(entry_${code}_example STREQUAL "")
            set(field "example")
        else()
            set(field "")
        endif()
    elseif(line MATCHES "^\\*\\*Mensagem:\\*\\* ?(.*)$")
        set(field "message")
        string(APPEND entry_${code}_message "${CMAKE_MATCH_1}\n")
    elseif(line MATCHES "^\\*\\*Quando acontece:\\*\\* ?(.*)$")
        set(field "when")
        string(APPEND entry_${code}_when "${CMAKE_MATCH_1}\n")
    elseif(line MATCHES "^\\*\\*Como corrigir:\\*\\* ?(.*)$")
        set(field "fix")
        string(APPEND entry_${code}_fix "${CMAKE_MATCH_1}\n")
    elseif(NOT field STREQUAL "")
        string(APPEND entry_${code}_${field} "${line}\n")
    endif()
endforeach()
finish_entry()

if(in_code_block)
    message(FATAL_ERROR "docs/errors.md: a code block is never closed")
endif()

function(cpp_literal value out_var)
    string(REPLACE "<<SPIKE_BS>>" "\\" value "${value}")
    string(REPLACE "<<SPIKE_SC>>" ";" value "${value}")
    string(REPLACE "<<SPIKE_LB>>" "[" value "${value}")
    string(REPLACE "<<SPIKE_RB>>" "]" value "${value}")
    string(FIND "${value}" ")spike_doc\"" clash)
    if(NOT clash EQUAL -1)
        message(FATAL_ERROR "docs/errors.md: text contains ')spike_doc\"'")
    endif()
    set(${out_var} "R\"spike_doc(${value})spike_doc\"" PARENT_SCOPE)
endfunction()

list(SORT codes)
set(table "")
foreach(code IN LISTS codes)
    string(APPEND table "    {\"${code}\",\n")
    foreach(name IN ITEMS title message when fix example)
        cpp_literal("${entry_${code}_${name}}" literal)
        string(APPEND table "     ${literal},\n")
    endforeach()
    string(APPEND table "    },\n")
endforeach()

file(WRITE "${OUTPUT}"
"#include \"spike/error_catalog.hpp\"

namespace spike {

const std::vector<ErrorEntry>& all_errors() {
    static const std::vector<ErrorEntry> entries = {
${table}    };
    return entries;
}

} // namespace spike
")
