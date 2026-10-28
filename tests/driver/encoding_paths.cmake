if(NOT DEFINED SPIKE OR NOT DEFINED WORK_DIR)
    message(FATAL_ERROR "usage: cmake -DSPIKE=<spike> -DWORK_DIR=<dir> -P encoding_paths.cmake")
endif()

set(root "${WORK_DIR}/pasta com espaço e acentuação")
file(REMOVE_RECURSE "${root}")
file(MAKE_DIRECTORY "${root}")
if(WIN32)
    set(exe ".exe")
else()
    set(exe "")
endif()

function(expect_success what code)
    if(NOT code EQUAL 0)
        message(FATAL_ERROR "${what} failed with ${code}:\n${ARGN}")
    endif()
endfunction()

function(run_program path expected)
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "executable '${path}' was not created")
    endif()
    execute_process(COMMAND "${path}" OUTPUT_VARIABLE out RESULT_VARIABLE code)
    string(REPLACE "\r" "" out "${out}")
    expect_success("running ${path}" "${code}")
    if(NOT out STREQUAL "${expected}")
        message(FATAL_ERROR "'${path}' printed '${out}', expected '${expected}'")
    endif()
endfunction()

file(WRITE "${root}/olá mundo.por"
    "algoritmo \"olá\"\ninicio\n  escreval(\"Olá, ação!\")\nfimalgoritmo\n")
execute_process(
    COMMAND "${SPIKE}" "${root}/olá mundo.por" -o "${root}/saída com espaço/meu prog"
    OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE code)
expect_success("compiling in an accented folder" "${code}" "${out}${err}")
run_program("${root}/saída com espaço/meu prog${exe}" "Olá, ação!\n")

execute_process(
    COMMAND "${SPIKE}" "${root}/olá mundo.por"
    OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE code)
expect_success("compiling with the default output" "${code}" "${out}${err}")
run_program("${root}/olá mundo${exe}" "Olá, ação!\n")

file(WRITE "${root}/círculo π.por" "algoritmo \"pi\"\ninicio\n  escreval(\"π\")\nfimalgoritmo\n")
execute_process(
    COMMAND "${SPIKE}" "${root}/círculo π.por" -o "${root}/π com espaço"
    OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE code)
expect_success("compiling a name outside Windows-1252" "${code}" "${out}${err}")
run_program("${root}/π com espaço${exe}" "π\n")

string(ASCII 227 a_tilde)
string(ASCII 231 c_cedilla)
string(ASCII 13 cr)
file(WRITE "${root}/ansi.por"
    "algoritmo \"ansi\"${cr}\ninicio${cr}\n  escreval(\"Situa${c_cedilla}${a_tilde}o\")${cr}\nfimalgoritmo${cr}\n")
execute_process(
    COMMAND "${SPIKE}" "${root}/ansi.por" -o "${root}/ansi"
    OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE code)
expect_success("compiling a Windows-1252 file" "${code}" "${out}${err}")
if(NOT err MATCHES "W010")
    message(FATAL_ERROR "expected W010 for a Windows-1252 file, got:\n${err}")
endif()
run_program("${root}/ansi${exe}" "Situação\n")

string(ASCII 1 control)
string(ASCII 255 high)
file(WRITE "${root}/quebrado.por" "algoritmo \"x\"\ninicio\n  escreval(\"${control}${high}\")\nfimalgoritmo\n")
execute_process(
    COMMAND "${SPIKE}" check "${root}/quebrado.por"
    OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE code)
if(NOT code EQUAL 1 OR NOT err MATCHES "E005")
    message(FATAL_ERROR "expected E005 and exit 1 for a broken file, got ${code}:\n${err}")
endif()

message(STATUS "accented paths, spaces, Windows-1252 and broken files behave")
