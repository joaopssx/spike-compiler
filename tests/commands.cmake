set(SPIKE_DEV_WORK_DIR "${CMAKE_BINARY_DIR}/dev_work")
configure_file("${CMAKE_SOURCE_DIR}/examples/hello.por" "${SPIKE_DEV_WORK_DIR}/hello.por" COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/tests/programs/invalid/e400_missing_entao.por"
               "${SPIKE_DEV_WORK_DIR}/broken.por" COPYONLY)

add_test(NAME dev_clean_previous_run
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${SPIKE_DEV_WORK_DIR}/hello.dev" "${SPIKE_DEV_WORK_DIR}/broken.dev")
set_tests_properties(dev_clean_previous_run PROPERTIES FIXTURES_SETUP dev_work_clean)

set(SPIKE_DEV_OUT "${SPIKE_DEV_WORK_DIR}/hello.dev")
spike_add_cli_test(dev_writes_every_phase
    "-DARGS=dev|${SPIKE_DEV_WORK_DIR}/hello.por"
    -DEXPECT_CODE=0
    "-DEXPECT_STDOUT=^$"
    "-DEXPECT_STDERR=tempo por etapa:.*xico.*sint.*tico.*sem.*ntico.*de C.*total.*ok: arquivos do modo dev em .*hello.dev.*6 arquivos"
    "-DEXPECT_FILE=${SPIKE_DEV_OUT}/tokens.txt|${SPIKE_DEV_OUT}/ast.txt|${SPIKE_DEV_OUT}/symbols.txt|${SPIKE_DEV_OUT}/types.txt|${SPIKE_DEV_OUT}/hello.c|${SPIKE_DEV_OUT}/times.txt"
)
set_tests_properties(dev_writes_every_phase PROPERTIES FIXTURES_REQUIRED dev_work_clean)

spike_add_cli_test(dev_error_leaves_no_folder
    "-DARGS=dev|${SPIKE_DEV_WORK_DIR}/broken.por"
    -DEXPECT_CODE=1
    "-DEXPECT_STDERR=erro\\[E400\\].*falhou: 1 erro"
    "-DEXPECT_NO_FILE=${SPIKE_DEV_WORK_DIR}/broken.dev"
)
set_tests_properties(dev_error_leaves_no_folder PROPERTIES FIXTURES_REQUIRED dev_work_clean)

spike_add_cli_test(random_is_planned
    "-DARGS=random"
    -DEXPECT_CODE=2
    "-DEXPECT_STDOUT=^$"
    "-DEXPECT_STDERR=ainda n.+o existe: o gerador de programas aleat.+rios est.+ planejado para a v1\\.0"
)

spike_add_cli_test(help_separates_planned_commands
    "-DARGS=--help"
    -DEXPECT_CODE=0
    "-DEXPECT_STDOUT=comandos:.*dev <arq>.*fetch.*planejados \\(ainda n.+o funcionam\\):.*random"
)

spike_add_cli_test(fetch_without_compiler_exits_3
    "-DARGS=fetch"
    -DEXPECT_CODE=3
    "-DEXPECT_STDOUT=Spike ${PROJECT_VERSION}.*compilador C: n.+o encontrado.*spike-no-such-cc.*falta:"
    ENV "NO_COLOR=1;SPIKE_CC=spike-no-such-cc"
)

if(SPIKE_TEST_C)
    spike_add_cli_test(fetch_with_compiler_reports_version
        "-DARGS=fetch"
        -DEXPECT_CODE=0
        "-DEXPECT_STDOUT=Spike ${PROJECT_VERSION}.*sistema: .*compilador C: .*do C: [^\n]*[0-9]+\\.[0-9]+.*SPIKE_CC: .*pasta tempor.+ria: grav.+vel.*tudo certo"
        ENV "NO_COLOR=1;SPIKE_CC=${SPIKE_TEST_C}"
    )
endif()

spike_add_cli_test(setup_check_without_compiler_exits_3
    "-DARGS=setup|--check"
    -DEXPECT_CODE=3
    "-DEXPECT_STDERR=nenhum compilador C encontrado: SPIKE_CC aponta para 'spike-no-such-cc'"
    ENV "NO_COLOR=1;SPIKE_CC=spike-no-such-cc"
)

spike_add_cli_test(setup_rejects_two_modes
    "-DARGS=setup|--check|--remove"
    -DEXPECT_CODE=2
    "-DEXPECT_STDERR=use s.+ uma destas flags"
    ENV "NO_COLOR=1"
)

if(SPIKE_TEST_C)
    spike_add_cli_test(setup_check_with_compiler_exits_0
        "-DARGS=setup|--check"
        -DEXPECT_CODE=0
        "-DEXPECT_STDERR=ok: o compilador C j.+ est.+ instalado: "
        ENV "NO_COLOR=1;SPIKE_CC=${SPIKE_TEST_C}"
    )
endif()
