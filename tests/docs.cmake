add_executable(spike_doctest tools/doctest.cpp)
target_link_libraries(spike_doctest PRIVATE spike_core spike_warnings)
spike_sanitize(spike_doctest)

set(spike_doctest_mode "")
set(spike_doctest_env "NO_COLOR=1;SPIKE_SEED=2026")
if(SPIKE_TEST_C)
    list(APPEND spike_doctest_env "SPIKE_CC=${SPIKE_TEST_C}")
else()
    set(spike_doctest_mode "--no-run")
endif()

add_test(NAME docs_examples
    COMMAND spike_doctest $<TARGET_FILE:spike> ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}/doctest_work ${spike_doctest_mode}
)
set_tests_properties(docs_examples PROPERTIES
    ENVIRONMENT "${spike_doctest_env}"
    PASS_REGULAR_EXPRESSION "docs examples: [0-9]+ passed, 0 failed"
)
