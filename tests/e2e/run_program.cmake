foreach(required SPIKE SOURCE EXE WORK_DIR NAME)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "${required} is required")
    endif()
endforeach()
if(NOT DEFINED EXPECT_CODE)
    set(EXPECT_CODE 0)
endif()

execute_process(
    COMMAND "${SPIKE}" "${SOURCE}" -o "${EXE}"
    RESULT_VARIABLE build_code
    OUTPUT_VARIABLE build_out
    ERROR_VARIABLE build_err
)
if(NOT build_code EQUAL 0)
    message(FATAL_ERROR "spike failed with code ${build_code}\n${build_out}${build_err}")
endif()

set(input_file "${WORK_DIR}/${NAME}.in")
if(DEFINED INPUT AND EXISTS "${INPUT}")
    set(input_file "${INPUT}")
else()
    file(WRITE "${input_file}" "")
endif()

execute_process(
    COMMAND "${EXE}${EXE_SUFFIX}"
    INPUT_FILE "${input_file}"
    RESULT_VARIABLE code
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
)
string(REPLACE "\r" "" out "${out}")
string(REPLACE "\r" "" err "${err}")

if(NOT code STREQUAL EXPECT_CODE)
    message(FATAL_ERROR "exit code ${code}, expected ${EXPECT_CODE}\nstdout:\n${out}\nstderr:\n${err}")
endif()
if(DEFINED EXPECTED_OUTPUT)
    file(READ "${EXPECTED_OUTPUT}" expected)
    string(REPLACE "\r" "" expected "${expected}")
    if(NOT out STREQUAL expected)
        message(FATAL_ERROR "stdout differs from ${EXPECTED_OUTPUT}\n--- expected\n${expected}--- got\n${out}")
    endif()
endif()
if(DEFINED EXPECT_STDERR AND NOT err MATCHES "${EXPECT_STDERR}")
    message(FATAL_ERROR "stderr '${err}' does not match '${EXPECT_STDERR}'")
endif()
