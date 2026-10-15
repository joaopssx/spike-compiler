file(GLOB SPIKE_VALID_PROGRAMS CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/examples/*.por"
    "${CMAKE_SOURCE_DIR}/tests/programs/valid/*.por"
)

foreach(program IN LISTS SPIKE_VALID_PROGRAMS)
    get_filename_component(program_name "${program}" NAME_WE)
    get_filename_component(program_dir "${program}" DIRECTORY)
    get_filename_component(program_group "${program_dir}" NAME)
    set(test_name "program_${program_group}_${program_name}")
    add_test(NAME "${test_name}" COMMAND spike tokens "${program}")
    set_tests_properties("${test_name}" PROPERTIES
        PASS_REGULAR_EXPRESSION "parse: OK"
    )
endforeach()

file(GLOB SPIKE_INVALID_PROGRAMS CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/tests/programs/invalid/*.por"
)

foreach(program IN LISTS SPIKE_VALID_PROGRAMS)
    get_filename_component(program_name "${program}" NAME_WE)
    get_filename_component(program_dir "${program}" DIRECTORY)
    get_filename_component(program_group "${program_dir}" NAME)
    set(test_name "check_${program_group}_${program_name}")
    add_test(NAME "${test_name}" COMMAND spike check "${program}")
    set_tests_properties("${test_name}" PROPERTIES PASS_REGULAR_EXPRESSION "nenhum erro encontrado")
endforeach()

file(GLOB SPIKE_SEMANTIC_INVALID_PROGRAMS CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/tests/programs/semantic_invalid/*.por"
)

foreach(program IN LISTS SPIKE_SEMANTIC_INVALID_PROGRAMS)
    get_filename_component(program_name "${program}" NAME_WE)
    string(REGEX MATCH "^[eE][0-9][0-9][0-9]" expected_code "${program_name}")
    if(NOT expected_code)
        message(FATAL_ERROR
            "invalid program '${program_name}.por' must start with an error code, e.g. e700_")
    endif()
    string(TOUPPER "${expected_code}" expected_code)
    set(test_name "check_semantic_invalid_${program_name}")
    add_test(NAME "${test_name}" COMMAND spike check "${program}")
    set_tests_properties("${test_name}" PROPERTIES
        PASS_REGULAR_EXPRESSION "\\[${expected_code}\\]"
    )
endforeach()

add_test(NAME compile_without_c_compiler
    COMMAND ${CMAKE_COMMAND}
        -DPROGRAM=$<TARGET_FILE:spike>
        "-DARGS=${CMAKE_SOURCE_DIR}/examples/hello.por|-o|${CMAKE_BINARY_DIR}/e2e/no_compiler"
        -DWORK_DIR=${CMAKE_BINARY_DIR}
        -DNAME=compile_without_c_compiler
        -DEXPECT_CODE=3
        "-DEXPECT_STDERR=E902.*SPIKE_CC"
        -P "${CMAKE_SOURCE_DIR}/tests/runtime/expect_run.cmake"
)
set_tests_properties(compile_without_c_compiler PROPERTIES
    ENVIRONMENT "SPIKE_CC=${CMAKE_BINARY_DIR}/no/such/compiler"
)

foreach(program IN LISTS SPIKE_INVALID_PROGRAMS)
    get_filename_component(program_name "${program}" NAME_WE)
    string(REGEX MATCH "^[eE][0-9][0-9][0-9]" expected_code "${program_name}")
    if(NOT expected_code)
        message(FATAL_ERROR
            "invalid program '${program_name}.por' must start with an error code, e.g. e400_")
    endif()
    string(TOUPPER "${expected_code}" expected_code)
    set(test_name "program_invalid_${program_name}")
    add_test(NAME "${test_name}" COMMAND spike tokens "${program}")
    set_tests_properties("${test_name}" PROPERTIES
        PASS_REGULAR_EXPRESSION "\\[${expected_code}\\]"
    )
endforeach()
