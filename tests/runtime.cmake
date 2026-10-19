include(CheckLanguage)
check_language(C)
if(NOT CMAKE_C_COMPILER)
    message(STATUS "No C compiler found: skipping C runtime tests")
    return()
endif()
enable_language(C)
target_compile_definitions(spike_tests PRIVATE SPIKE_TEST_C_COMPILER="${CMAKE_C_COMPILER}")

function(spike_add_runtime_program target source)
    add_executable(${target} ${source})
    target_include_directories(${target} PRIVATE "${CMAKE_SOURCE_DIR}/runtime")
    target_link_libraries(${target} PRIVATE spike_warnings)
    if(UNIX)
        target_link_libraries(${target} PRIVATE m)
    endif()
    set_target_properties(${target} PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF
    )
endfunction()

spike_add_runtime_program(spike_runtime_test tests/runtime/runtime_test.c)
add_test(NAME runtime_c_unit COMMAND spike_runtime_test)
set_tests_properties(runtime_c_unit PROPERTIES PASS_REGULAR_EXPRESSION "runtime tests passed")

spike_add_runtime_program(spike_runtime_io tests/runtime/runtime_io_program.c)

function(spike_add_runtime_io_test name mode input code)
    add_test(NAME runtime_io_${name}
        COMMAND ${CMAKE_COMMAND}
            -DPROGRAM=$<TARGET_FILE:spike_runtime_io>
            -DARGS=${mode}
            "-DINPUT_TEXT=${input}"
            -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}
            -DNAME=runtime_io_${name}
            -DEXPECT_CODE=${code}
            ${ARGN}
            -P "${CMAKE_SOURCE_DIR}/tests/runtime/expect_run.cmake"
    )
endfunction()

spike_add_runtime_io_test(read_int int "42\\n" 0 "-DEXPECT_STDOUT=^42$")
spike_add_runtime_io_test(read_int_spaces int "  -7  \\r\\n" 0 "-DEXPECT_STDOUT=^-7$")
spike_add_runtime_io_test(read_real_comma real "3,5\\n" 0 "-DEXPECT_STDOUT=^3.5$")
spike_add_runtime_io_test(read_real_whole real "7\\n" 0 "-DEXPECT_STDOUT=^7$")
spike_add_runtime_io_test(read_bool_sim bool "Sim\\n" 0 "-DEXPECT_STDOUT=^verdadeiro$")
spike_add_runtime_io_test(read_str_crlf str "ola mundo\\r\\n" 0 "-DEXPECT_STDOUT=^ola mundo$")
spike_add_runtime_io_test(bad_int int "abc\\n" 101
    "-DEXPECT_STDERR=R003\\]: esperava um n.+mero inteiro, mas recebeu 'abc'.*linha 5")
spike_add_runtime_io_test(empty_int int "\\n" 101 "-DEXPECT_STDERR=R003.*uma linha vazia")
spike_add_runtime_io_test(long_bad_real real "0123456789012345678901234567890123456789XYZ\\n" 101
    "-DEXPECT_STDERR=R003.*recebeu '0123456789012345678901234567890123456789[.][.][.]'")
spike_add_runtime_io_test(bad_bool bool "talvez\\n" 101 "-DEXPECT_STDERR=R003")
spike_add_runtime_io_test(end_of_input int "" 101
    "-DEXPECT_STDERR=R007\\]: a entrada acabou antes de leia\\(\\) conseguir ler 'x'.*linha 5")
spike_add_runtime_io_test(div_by_zero div "" 101
    "-DEXPECT_STDERR=R001\\]: divis.+o por zero.*linha 7")
spike_add_runtime_io_test(div_real_by_zero divreal "" 101 "-DEXPECT_STDERR=R001.*linha 9")
spike_add_runtime_io_test(pow_infinite pow "" 101
    "-DEXPECT_STDERR=R005\\]: resultado inv.+lido na pot.+ncia.*linha 11")
spike_add_runtime_io_test(pow_not_a_number powneg "" 101 "-DEXPECT_STDERR=R005.*linha 12")
spike_add_runtime_io_test(index_out_of_range index "" 101
    "-DEXPECT_STDERR=R002\\]: .+ndice 11 fora do vetor 'notas' \\(v.+lido: 1 a 5\\).*linha 13")
spike_add_runtime_io_test(index_out_of_dimension index2 "" 101
    "-DEXPECT_STDERR=R002\\]: .+ndice 5 fora da dimens.+o 2 de 'm' \\(v.+lido: 1 a 4\\).*linha 14")
spike_add_runtime_io_test(call_depth depth "" 101
    "-DEXPECT_STDERR=R008\\]: recurs.+o profunda demais \\(limite de 10000 chamadas\\).*linha 21")
spike_add_runtime_io_test(missing_return noreturn "" 101
    "-DEXPECT_STDERR=R009\\]: a fun.+o 'sinal' terminou sem retornar um valor.*linha 22")
