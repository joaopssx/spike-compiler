if(NOT CMAKE_C_COMPILER)
    message(STATUS "No C compiler found: skipping compile and run tests")
    return()
endif()

set(SPIKE_E2E_DIR "${CMAKE_BINARY_DIR}/e2e")
file(MAKE_DIRECTORY "${SPIKE_E2E_DIR}")

function(spike_add_expect_test name)
    add_test(NAME ${name}
        COMMAND ${CMAKE_COMMAND}
            -DWORK_DIR=${SPIKE_E2E_DIR}
            -DNAME=${name}
            ${ARGN}
            -P "${CMAKE_SOURCE_DIR}/tests/runtime/expect_run.cmake"
    )
    set_tests_properties(${name} PROPERTIES ENVIRONMENT "SPIKE_CC=${CMAKE_C_COMPILER}")
endfunction()

if(WIN32)
    set(SPIKE_EXE_SUFFIX ".exe")
else()
    set(SPIKE_EXE_SUFFIX "")
endif()

spike_add_expect_test(compile_hello
    -DPROGRAM=$<TARGET_FILE:spike>
    "-DARGS=${CMAKE_SOURCE_DIR}/examples/hello.por|-o|${SPIKE_E2E_DIR}/hello_test"
    -DEXPECT_CODE=0
    "-DEXPECT_STDOUT=ok:.*gerado.*hello_test"
    "-DEXPECT_FILE=${SPIKE_E2E_DIR}/hello_test${SPIKE_EXE_SUFFIX}"
)
set_tests_properties(compile_hello PROPERTIES FIXTURES_SETUP hello_executable)

spike_add_expect_test(run_compiled_hello
    "-DPROGRAM=${SPIKE_E2E_DIR}/hello_test${SPIKE_EXE_SUFFIX}"
    -DEXPECT_CODE=0
    "-DEXPECT_STDOUT=^Ol.+, mundo!$"
)
set_tests_properties(run_compiled_hello PROPERTIES FIXTURES_REQUIRED hello_executable)

spike_add_expect_test(verbose_lists_every_step
    -DPROGRAM=$<TARGET_FILE:spike>
    "-DARGS=${CMAKE_SOURCE_DIR}/examples/hello.por|--verbose|-o|${SPIKE_E2E_DIR}/hello_verbose"
    -DEXPECT_CODE=0
    "-DEXPECT_STDERR=1/5.*lendo arquivo.*2/5.*tokens.*3/5.*1 comando.*4/5.*0 vari.*5/5.*gerando C.*compilador C: "
)

spike_add_expect_test(time_lists_every_phase
    -DPROGRAM=$<TARGET_FILE:spike>
    "-DARGS=${CMAKE_SOURCE_DIR}/examples/hello.por|--time|-o|${SPIKE_E2E_DIR}/hello_time"
    -DEXPECT_CODE=0
    "-DEXPECT_STDERR=tempo por etapa:.*xico.*sint.*tico.*sem.*ntico.*de C.*compilador C.*total.* ms"
)

spike_add_expect_test(compile_and_run_hello
    -DPROGRAM=$<TARGET_FILE:spike>
    "-DARGS=${CMAKE_SOURCE_DIR}/examples/hello.por|--run|-o|${SPIKE_E2E_DIR}/hello_run"
    -DEXPECT_CODE=0
    "-DEXPECT_STDOUT=ok:.*gerado.*Ol.+, mundo!$"
)

spike_add_expect_test(compile_keeps_c_file
    -DPROGRAM=$<TARGET_FILE:spike>
    "-DARGS=${CMAKE_SOURCE_DIR}/examples/hello.por|--c|-o|${SPIKE_E2E_DIR}/hello_c"
    -DEXPECT_CODE=0
    "-DEXPECT_FILE=${SPIKE_E2E_DIR}/hello_c.c"
)

spike_add_expect_test(run_exit_code_is_forwarded
    -DPROGRAM=$<TARGET_FILE:spike>
    "-DARGS=${CMAKE_SOURCE_DIR}/tests/programs/runtime/r001_division_by_zero.por|--run|-o|${SPIKE_E2E_DIR}/divide"
    -DEXPECT_CODE=101
    "-DEXPECT_STDERR=R001.*linha 5"
)

spike_add_expect_test(compile_reports_semantic_errors
    -DPROGRAM=$<TARGET_FILE:spike>
    "-DARGS=${CMAKE_SOURCE_DIR}/tests/programs/semantic_invalid/e700_undeclared.por|-o|${SPIKE_E2E_DIR}/never"
    -DEXPECT_CODE=1
    "-DEXPECT_STDERR=E700"
)

file(GLOB SPIKE_RUN_PROGRAMS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tests/programs/run/*.por")
foreach(program IN LISTS SPIKE_RUN_PROGRAMS)
    get_filename_component(name "${program}" NAME_WE)
    get_filename_component(dir "${program}" DIRECTORY)
    if(NOT EXISTS "${dir}/${name}.out")
        message(FATAL_ERROR "tests/programs/run/${name}.por needs ${name}.out with the expected output")
    endif()
    add_test(NAME run_${name}
        COMMAND ${CMAKE_COMMAND}
            -DSPIKE=$<TARGET_FILE:spike>
            "-DSOURCE=${program}"
            "-DEXE=${SPIKE_E2E_DIR}/run_${name}"
            "-DEXE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX}"
            "-DEXPECTED_OUTPUT=${dir}/${name}.out"
            "-DINPUT=${dir}/${name}.in"
            -DWORK_DIR=${SPIKE_E2E_DIR}
            -DNAME=run_${name}
            -P "${CMAKE_SOURCE_DIR}/tests/e2e/run_program.cmake"
    )
    set_tests_properties(run_${name} PROPERTIES ENVIRONMENT "SPIKE_CC=${CMAKE_C_COMPILER}")
endforeach()

file(GLOB SPIKE_RUNTIME_ERROR_PROGRAMS CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/tests/programs/runtime/*.por"
)
foreach(program IN LISTS SPIKE_RUNTIME_ERROR_PROGRAMS)
    get_filename_component(name "${program}" NAME_WE)
    string(REGEX MATCH "^[rR][0-9][0-9][0-9]" expected_code "${name}")
    if(NOT expected_code)
        message(FATAL_ERROR "runtime program '${name}.por' must start with its code, e.g. r001_")
    endif()
    string(TOUPPER "${expected_code}" expected_code)
    add_test(NAME runtime_error_${name}
        COMMAND ${CMAKE_COMMAND}
            -DSPIKE=$<TARGET_FILE:spike>
            "-DSOURCE=${program}"
            "-DEXE=${SPIKE_E2E_DIR}/runtime_${name}"
            "-DEXE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX}"
            -DEXPECT_CODE=101
            "-DEXPECT_STDERR=\\[${expected_code}\\].*linha [0-9]+"
            "-DINPUT=${CMAKE_SOURCE_DIR}/tests/programs/runtime/${name}.in"
            -DWORK_DIR=${SPIKE_E2E_DIR}
            -DNAME=runtime_${name}
            -P "${CMAKE_SOURCE_DIR}/tests/e2e/run_program.cmake"
    )
    set_tests_properties(runtime_error_${name} PROPERTIES ENVIRONMENT "SPIKE_CC=${CMAKE_C_COMPILER}")
endforeach()