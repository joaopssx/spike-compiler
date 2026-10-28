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

add_test(NAME ast_flag_prints_tree_and_does_not_compile
    COMMAND ${CMAKE_COMMAND}
        -DPROGRAM=$<TARGET_FILE:spike>
        "-DARGS=${CMAKE_SOURCE_DIR}/examples/sum.por|--ast|-o|${CMAKE_BINARY_DIR}/e2e/ast_only"
        -DWORK_DIR=${CMAKE_BINARY_DIR}
        -DNAME=ast_flag
        -DEXPECT_CODE=0
        "-DEXPECT_STDOUT=^Program \"soma\" \\(2:1\\).*Leia a \\(7:3\\)"
        "-DEXPECT_NO_FILE=${CMAKE_BINARY_DIR}/e2e/ast_only${CMAKE_EXECUTABLE_SUFFIX}"
        -P "${CMAKE_SOURCE_DIR}/tests/runtime/expect_run.cmake"
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

file(GLOB SPIKE_WARNING_PROGRAMS CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/tests/programs/warnings/*.por"
)

foreach(program IN LISTS SPIKE_WARNING_PROGRAMS)
    get_filename_component(program_name "${program}" NAME_WE)
    string(REGEX MATCH "^[wW][0-9][0-9][0-9]" expected_code "${program_name}")
    if(NOT expected_code)
        message(FATAL_ERROR
            "warning program '${program_name}.por' must start with a warning code, e.g. w001_")
    endif()
    string(TOUPPER "${expected_code}" expected_code)
    set(test_name "check_warning_${program_name}")
    add_test(NAME "${test_name}"
        COMMAND ${CMAKE_COMMAND}
            -DPROGRAM=$<TARGET_FILE:spike>
            "-DARGS=check|${program}"
            -DWORK_DIR=${CMAKE_BINARY_DIR}
            -DNAME=${test_name}
            -DEXPECT_CODE=0
            "-DEXPECT_STDERR=aviso.${expected_code}..*nenhum erro encontrado"
            -P "${CMAKE_SOURCE_DIR}/tests/runtime/expect_run.cmake"
    )
endforeach()
