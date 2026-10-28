if(NOT SPIKE_TEST_C)
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
    set_tests_properties(${name} PROPERTIES ENVIRONMENT "SPIKE_CC=${SPIKE_TEST_C}")
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
    "-DEXPECT_STDOUT=^$"
    "-DEXPECT_STDERR=ok:.*gerado.*hello_test"
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
    "-DEXPECT_STDOUT=^Ol.+, mundo!$"
    "-DEXPECT_STDERR=ok:.*gerado"
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
    "-DEXPECT_STDERR=R001.*program.por:8"
)

spike_add_expect_test(compile_reports_semantic_errors
    -DPROGRAM=$<TARGET_FILE:spike>
    "-DARGS=${CMAKE_SOURCE_DIR}/tests/programs/semantic_invalid/e700_undeclared.por|-o|${SPIKE_E2E_DIR}/never"
    -DEXPECT_CODE=1
    "-DEXPECT_STDERR=E700.*falhou: 1 erro.Para entender um erro: spike --explain E700"
)

set(SPIKE_WARNING_SAMPLE "${CMAKE_SOURCE_DIR}/tests/programs/warnings/w001_unused_variable.por")

spike_add_expect_test(warnings_are_shown_on_success
    -DPROGRAM=$<TARGET_FILE:spike>
    "-DARGS=${SPIKE_WARNING_SAMPLE}|-o|${SPIKE_E2E_DIR}/warning_shown"
    -DEXPECT_CODE=0
    "-DEXPECT_STDERR=aviso.W001..*idade.*ok:.*gerado"
    "-DEXPECT_FILE=${SPIKE_E2E_DIR}/warning_shown${SPIKE_EXE_SUFFIX}"
)

spike_add_expect_test(no_warnings_flag_hides_warnings
    -DPROGRAM=$<TARGET_FILE:spike>
    "-DARGS=${SPIKE_WARNING_SAMPLE}|--no-warnings|-o|${SPIKE_E2E_DIR}/warning_hidden"
    -DEXPECT_CODE=0
    "-DEXPECT_STDERR=^ok:.*gerado"
)

spike_add_expect_test(warnings_as_errors_stops_compilation
    -DPROGRAM=$<TARGET_FILE:spike>
    "-DARGS=${SPIKE_WARNING_SAMPLE}|--warnings-as-errors|-o|${SPIKE_E2E_DIR}/warning_fails"
    -DEXPECT_CODE=1
    "-DEXPECT_STDERR=aviso.W001..*1 aviso e '--warnings-as-errors'"
    "-DEXPECT_NO_FILE=${SPIKE_E2E_DIR}/warning_fails${SPIKE_EXE_SUFFIX}"
)

spike_add_expect_test(warnings_as_errors_wins_over_no_warnings
    -DPROGRAM=$<TARGET_FILE:spike>
    "-DARGS=${SPIKE_WARNING_SAMPLE}|--no-warnings|--warnings-as-errors|-o|${SPIKE_E2E_DIR}/warning_both"
    -DEXPECT_CODE=1
    "-DEXPECT_STDERR=aviso.W001."
)

file(GLOB SPIKE_EXAMPLE_PROGRAMS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/examples/*.por")
foreach(program IN LISTS SPIKE_EXAMPLE_PROGRAMS)
    get_filename_component(program_name "${program}" NAME_WE)
    spike_add_expect_test(example_without_warnings_${program_name}
        -DPROGRAM=$<TARGET_FILE:spike>
        "-DARGS=${program}|--warnings-as-errors|-o|${SPIKE_E2E_DIR}/examples/${program_name}"
        -DEXPECT_CODE=0
    )
endforeach()

add_test(NAME compile_accented_paths_and_encodings
    COMMAND ${CMAKE_COMMAND} -DSPIKE=$<TARGET_FILE:spike> -DWORK_DIR=${SPIKE_E2E_DIR}
        -P ${CMAKE_SOURCE_DIR}/tests/driver/encoding_paths.cmake
)
set_tests_properties(compile_accented_paths_and_encodings PROPERTIES
    ENVIRONMENT "SPIKE_CC=${SPIKE_TEST_C}")
