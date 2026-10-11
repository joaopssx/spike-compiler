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
