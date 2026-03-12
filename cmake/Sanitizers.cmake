option(SPIKE_ENABLE_SANITIZERS "Enable address and undefined sanitizers" OFF)

function(spike_enable_sanitizers target_name)
    if(SPIKE_ENABLE_SANITIZERS AND NOT MSVC)
        target_compile_options(${target_name} PRIVATE -fsanitize=address,undefined)
        target_link_options(${target_name} PRIVATE -fsanitize=address,undefined)
    endif()
endfunction()
