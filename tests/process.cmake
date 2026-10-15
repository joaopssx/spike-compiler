add_executable(spike_echo_args tests/process/echo_args.cpp)
target_link_libraries(spike_echo_args PRIVATE spike_warnings)
if(WIN32)
    target_link_libraries(spike_echo_args PRIVATE shell32)
endif()

add_executable(spike_fake_cc tests/process/fake_cc.cpp)
target_link_libraries(spike_fake_cc PRIVATE spike_warnings)

add_dependencies(spike_tests spike_echo_args spike_fake_cc)
add_test(NAME process_and_driver COMMAND spike_tests process cc_driver)
set_tests_properties(process_and_driver PROPERTIES PASS_REGULAR_EXPRESSION "all tests passed")
target_compile_definitions(spike_tests PRIVATE
    SPIKE_ECHO_ARGS="$<TARGET_FILE:spike_echo_args>"
    SPIKE_FAKE_CC="$<TARGET_FILE:spike_fake_cc>"
)
