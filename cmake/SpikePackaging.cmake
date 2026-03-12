include(GNUInstallDirs)

function(spike_install_target target_name)
    install(TARGETS ${target_name} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endfunction()
