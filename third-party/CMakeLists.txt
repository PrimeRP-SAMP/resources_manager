add_subdirectory(curl)
add_subdirectory(zstd)
add_subdirectory(json)
if (NOT ANDROID)
    add_subdirectory(glog)
else ()
    add_subdirectory(fmt)
endif ()

macro(include_third_party name)
    include_curl(${name})
    include_zstd(${name})
    include_json(${name})
    if (NOT ANDROID)
        include_glog(${name})
    endif ()
endmacro()

macro(link_third_party name)
    link_curl(${name})
    link_zstd(${name})
    if (NOT ANDROID)
        link_glog(${name})
    endif ()
endmacro()

if (ANDROID)
    macro(prepare_fmt name)
        target_link_libraries(${name} fmt::fmt)
        add_custom_command(
                TARGET ${name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_LINKER_FILE:fmt::fmt> $<TARGET_FILE_DIR:${name}>
        )
    endmacro()
endif ()
