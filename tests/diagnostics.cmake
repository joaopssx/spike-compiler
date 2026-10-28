set(SPIKE_E400_PROGRAM "${CMAKE_SOURCE_DIR}/tests/programs/invalid/e400_missing_entao.por")
set(SPIKE_NO_COLOR_ENV "NO_COLOR=;FORCE_COLOR=")

function(spike_add_cli_test name)
    cmake_parse_arguments(PARSE_ARGV 1 cli "" "ENV" "")
    add_test(NAME ${name}
        COMMAND ${CMAKE_COMMAND}
            -DPROGRAM=$<TARGET_FILE:spike>
            -DWORK_DIR=${CMAKE_BINARY_DIR}
            -DNAME=${name}
            ${cli_UNPARSED_ARGUMENTS}
            -P "${CMAKE_SOURCE_DIR}/tests/runtime/expect_run.cmake"
    )
    if(cli_ENV)
        set_tests_properties(${name} PROPERTIES ENVIRONMENT "${cli_ENV}")
    else()
        set_tests_properties(${name} PROPERTIES ENVIRONMENT "${SPIKE_NO_COLOR_ENV}")
    endif()
endfunction()

spike_add_cli_test(diagnostics_go_to_stderr_with_summary
    "-DARGS=check|${SPIKE_E400_PROGRAM}"
    -DEXPECT_CODE=1
    "-DEXPECT_STDOUT=^$"
    "-DEXPECT_STDERR=erro\\[E400\\].*falhou: 1 erro.Para entender um erro: spike --explain E400.$"
    -DEXPECT_COLOR=0
)

spike_add_cli_test(check_success_goes_to_stderr
    "-DARGS=check|${CMAKE_SOURCE_DIR}/examples/hello.por"
    -DEXPECT_CODE=0
    "-DEXPECT_STDOUT=^$"
    "-DEXPECT_STDERR=^ok: nenhum erro encontrado.$"
)

spike_add_cli_test(ast_errors_end_with_summary
    "-DARGS=${SPIKE_E400_PROGRAM}|--ast"
    -DEXPECT_CODE=1
    "-DEXPECT_STDOUT=^$"
    "-DEXPECT_STDERR=falhou: 1 erro."
)

spike_add_cli_test(help_goes_to_stdout
    "-DARGS=--help"
    -DEXPECT_CODE=0
    "-DEXPECT_STDOUT=--explain.*--color=always"
    "-DEXPECT_STDERR=^$"
)

spike_add_cli_test(no_color_env_removes_colors
    "-DARGS=check|${SPIKE_E400_PROGRAM}"
    -DEXPECT_CODE=1
    -DEXPECT_COLOR=0
    ENV "NO_COLOR=1;FORCE_COLOR=1"
)

spike_add_cli_test(color_always_adds_colors
    "-DARGS=check|${SPIKE_E400_PROGRAM}|--color=always"
    -DEXPECT_CODE=1
    -DEXPECT_COLOR=1
    ENV "NO_COLOR=1;FORCE_COLOR="
)

spike_add_cli_test(force_color_adds_colors
    "-DARGS=check|${SPIKE_E400_PROGRAM}"
    -DEXPECT_CODE=1
    -DEXPECT_COLOR=1
    ENV "NO_COLOR=;FORCE_COLOR=1"
)

spike_add_cli_test(color_never_beats_force_color
    "-DARGS=--color=never|check|${SPIKE_E400_PROGRAM}"
    -DEXPECT_CODE=1
    -DEXPECT_COLOR=0
    ENV "NO_COLOR=;FORCE_COLOR=1"
)

spike_add_cli_test(color_flag_rejects_bad_value
    "-DARGS=check|${SPIKE_E400_PROGRAM}|--color=talvez"
    -DEXPECT_CODE=2
    "-DEXPECT_STDERR=auto, always ou never"
)

spike_add_cli_test(explain_prints_the_entry
    "-DARGS=--explain|E400"
    -DEXPECT_CODE=0
    "-DEXPECT_STDOUT=^E400 .+ falta 'entao'..Mensagem: .*Quando acontece: .*Como corrigir: .*Exemplo:.    algoritmo"
    "-DEXPECT_STDERR=^$"
    -DEXPECT_COLOR=0
)

spike_add_cli_test(explain_ignores_case
    "-DARGS=--explain|e400"
    -DEXPECT_CODE=0
    "-DEXPECT_STDOUT=^E400 .+ falta 'entao'"
)

spike_add_cli_test(explain_colors_the_title
    "-DARGS=--explain|E400|--color=always"
    -DEXPECT_CODE=0
    -DEXPECT_COLOR=1
)

spike_add_cli_test(explain_unknown_code
    "-DARGS=--explain|E999"
    -DEXPECT_CODE=2
    "-DEXPECT_STDOUT=^$"
    "-DEXPECT_STDERR=n.+o conhe.+o o c.+digo E999. c.+digos v.+lidos: E001-E005, "
)

spike_add_cli_test(explain_lists_every_code
    "-DARGS=--explain"
    -DEXPECT_CODE=0
    "-DEXPECT_STDOUT=  E001  .*  E400  .*  E904  "
)
