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

spike_add_expect_test(compile_creates_output_folder
    -DPROGRAM=$<TARGET_FILE:spike>
    "-DARGS=${CMAKE_SOURCE_DIR}/examples/hello.por|--c|-o|${SPIKE_E2E_DIR}/nova/pasta/hello"
    -DEXPECT_CODE=0
    "-DEXPECT_FILE=${SPIKE_E2E_DIR}/nova/pasta/hello${SPIKE_EXE_SUFFIX}"
)

spike_add_expect_test(compile_keeps_c_file
    -DPROGRAM=$<TARGET_FILE:spike>
    "-DARGS=${CMAKE_SOURCE_DIR}/examples/hello.por|--c|-o|${SPIKE_E2E_DIR}/hello_c"
    -DEXPECT_CODE=0
    "-DEXPECT_FILE=${SPIKE_E2E_DIR}/hello_c.c"
)

spike_add_expect_test(run_exit_code_is_forwarded
    -DPROGRAM=$<TARGET_FILE:spike>
    "-DARGS=${CMAKE_SOURCE_DIR}/tests/e2e/integer_division_by_zero/program.por|--run|-o|${SPIKE_E2E_DIR}/divide"
    -DEXPECT_CODE=101
    "-DEXPECT_STDERR=R001.*linha 8"
)

spike_add_expect_test(compile_reports_semantic_errors
    -DPROGRAM=$<TARGET_FILE:spike>
    "-DARGS=${CMAKE_SOURCE_DIR}/tests/programs/semantic_invalid/e700_undeclared.por|-o|${SPIKE_E2E_DIR}/never"
    -DEXPECT_CODE=1
    "-DEXPECT_STDERR=E700"
)
