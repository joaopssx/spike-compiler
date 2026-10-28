set(SPIKE_TEST_CC "" CACHE FILEPATH
    "C compiler for the tests that compile generated C (default: the C compiler CMake finds)")

include(CheckLanguage)
check_language(C)
if(CMAKE_C_COMPILER)
    enable_language(C)
endif()

# The Spike driver speaks GCC and Clang flags, so MSVC's cl.exe cannot run these tests.
set(SPIKE_TEST_C "")
if(SPIKE_TEST_CC)
    file(TO_CMAKE_PATH "${SPIKE_TEST_CC}" SPIKE_TEST_C)
elseif(CMAKE_C_COMPILER AND NOT CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(SPIKE_TEST_C "${CMAKE_C_COMPILER}")
endif()
if(SPIKE_TEST_C)
    target_compile_definitions(spike_tests PRIVATE SPIKE_TEST_C_COMPILER="${SPIKE_TEST_C}")
    message(STATUS "C compiler for generated-C tests: ${SPIKE_TEST_C}")
endif()

if(NOT CMAKE_C_COMPILER OR CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "No GCC or Clang C compiler for CMake: skipping C runtime tests")
    return()
endif()

function(spike_add_runtime_program target source)
    add_executable(${target} ${source})
    target_include_directories(${target} PRIVATE "${CMAKE_SOURCE_DIR}/runtime")
    target_link_libraries(${target} PRIVATE spike_warnings)
    spike_sanitize(${target})
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

spike_add_runtime_program(spike_runtime_strings_test tests/runtime/runtime_strings_test.c)
add_test(NAME runtime_c_strings COMMAND spike_runtime_strings_test)
set_tests_properties(runtime_c_strings PROPERTIES PASS_REGULAR_EXPRESSION "string tests passed")

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
spike_add_runtime_io_test(error_shows_source source "" 101
    "-DEXPECT_STDERR=^erro de execu.+o\\[R001\\]: divis.+o por zero\n --> soma.por:3\n     [|]\n   3 [|]   r <- a div b\n     [|]\n  chamado de 'media' na linha 2\n     = dica: verifique se o divisor")
spike_add_runtime_io_test(error_line_outside_source sourcefar "" 101
    "-DEXPECT_STDERR=--> soma.por:9\n     = dica:")
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
spike_add_runtime_io_test(sqrt_negative sqrtneg "" 101
    "-DEXPECT_STDERR=R005\\]: raiz de n.+mero negativo.*linha 31")
spike_add_runtime_io_test(log_not_positive logzero "" 101
    "-DEXPECT_STDERR=R005\\]: logaritmo de n.+mero que n.+o .+ positivo.*linha 32")
spike_add_runtime_io_test(real_too_big_for_int bigint "" 101
    "-DEXPECT_STDERR=R004\\]: n.+mero grande demais para virar inteiro.*linha 33")
spike_add_runtime_io_test(nan_to_int nanint "" 101 "-DEXPECT_STDERR=R004.*linha 34")
spike_add_runtime_io_test(random_range randrange "" 101
    "-DEXPECT_STDERR=R010\\]: aleatorio\\(5, 1\\): o primeiro n.+mero precisa ser menor ou igual ao segundo.*linha 35")
spike_add_runtime_io_test(random_seed seeded "" 0 "-DEXPECT_STDOUT=^275414$")
set_tests_properties(runtime_io_random_seed PROPERTIES ENVIRONMENT "SPIKE_SEED=42")
spike_add_runtime_io_test(slice_start_zero slicezero "" 101
    "-DEXPECT_STDERR=R011\\]: copia: a posi.+o inicial precisa ser 1 ou mais, mas .+ 0.*linha 41")
spike_add_runtime_io_test(ord_empty ordempty "" 101 "-DEXPECT_STDERR=R011\\]: ord: o texto est.+ vazio.*linha 42")
spike_add_runtime_io_test(chr_zero chrzero "" 101
    "-DEXPECT_STDERR=R011\\]: chr: 0 n.+o .+ o c.+digo de um caractere \\(v.+lido: 1 a 1114111\\).*linha 43")
spike_add_runtime_io_test(chr_too_big chrbig "" 101 "-DEXPECT_STDERR=R011.*linha 44")
spike_add_runtime_io_test(text_to_int toint "" 101
    "-DEXPECT_STDERR=R004\\]: n.+o foi poss.+vel converter 'abc' para inteiro.*linha 45")
spike_add_runtime_io_test(empty_to_real toreal "" 101
    "-DEXPECT_STDERR=R004\\]: n.+o foi poss.+vel converter um texto vazio para real.*linha 46")
