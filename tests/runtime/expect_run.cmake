if(NOT DEFINED PROGRAM OR NOT DEFINED WORK_DIR OR NOT DEFINED NAME)
    message(FATAL_ERROR "PROGRAM, WORK_DIR and NAME are required")
endif()

set(input_text "${INPUT_TEXT}")
string(REPLACE "\\r" "\r" input_text "${input_text}")
string(REPLACE "\\n" "\n" input_text "${input_text}")
set(input_file "${WORK_DIR}/${NAME}.in")
file(WRITE "${input_file}" "${input_text}")

string(REPLACE "|" ";" args "${ARGS}")

execute_process(
    COMMAND "${PROGRAM}" ${args}
    INPUT_FILE "${input_file}"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
    RESULT_VARIABLE code
)

string(REPLACE "\r" "" out "${out}")
string(REPLACE "\r" "" err "${err}")
string(REGEX REPLACE "\n$" "" out "${out}")

if(DEFINED EXPECT_CODE AND NOT code STREQUAL EXPECT_CODE)
    message(FATAL_ERROR "exit code ${code}, expected ${EXPECT_CODE}\nstdout: ${out}\nstderr: ${err}")
endif()
if(DEFINED EXPECT_STDOUT AND NOT out MATCHES "${EXPECT_STDOUT}")
    message(FATAL_ERROR "stdout '${out}' does not match '${EXPECT_STDOUT}'")
endif()
if(DEFINED EXPECT_STDERR AND NOT err MATCHES "${EXPECT_STDERR}")
    message(FATAL_ERROR "stderr '${err}' does not match '${EXPECT_STDERR}'")
endif()
if(DEFINED EXPECT_FILE AND NOT EXISTS "${EXPECT_FILE}")
    message(FATAL_ERROR "expected file '${EXPECT_FILE}' was not created")
endif()
