add_executable(spike_e2e
    tests/e2e/runner/main.cpp
    tests/e2e/runner/compare.cpp
)
target_link_libraries(spike_e2e PRIVATE spike_core spike_warnings)

set(SPIKE_E2E_WORK "${CMAKE_BINARY_DIR}/e2e_work")

function(spike_add_e2e_cases folder prefix)
    file(GLOB entries LIST_DIRECTORIES true CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/${folder}/*")
    foreach(entry IN LISTS entries)
        if(NOT IS_DIRECTORY "${entry}" OR NOT EXISTS "${entry}/program.por")
            continue()
        endif()
        get_filename_component(name "${entry}" NAME)
        set(test_name "${prefix}${name}")
        add_test(NAME ${test_name}
            COMMAND spike_e2e $<TARGET_FILE:spike> "${entry}" "${SPIKE_E2E_WORK}/${test_name}"
        )
        set_tests_properties(${test_name} PROPERTIES
            LABELS e2e
            SKIP_RETURN_CODE 77
            TIMEOUT 60
        )
        set(environment "SPIKE_SEED=2026")
        if(CMAKE_C_COMPILER)
            list(APPEND environment "SPIKE_CC=${CMAKE_C_COMPILER}")
        endif()
        set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "${environment}")
    endforeach()
endfunction()

spike_add_e2e_cases(tests/e2e e2e_)
spike_add_e2e_cases(tests/e2e_errors e2e_error_)
