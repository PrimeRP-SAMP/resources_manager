set(ZSTD_LIB ${CMAKE_CURRENT_SOURCE_DIR}/${RM_ARCH}/Release/zstd.lib CACHE INTERNAL "")
set(ZSTD_DLL ${CMAKE_CURRENT_SOURCE_DIR}/${RM_ARCH}/Release/zstd.dll CACHE INTERNAL "")

set(ZSTD_DEBUG_LIB ${CMAKE_CURRENT_SOURCE_DIR}/${RM_ARCH}/Debug/zstd.lib CACHE INTERNAL "")
set(ZSTD_DEBUG_DLL ${CMAKE_CURRENT_SOURCE_DIR}/${RM_ARCH}/Debug/zstd.dll CACHE INTERNAL "")

set(ZSTD_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "")

add_library(zstd SHARED IMPORTED GLOBAL)
set_property(TARGET zstd PROPERTY IMPORTED_IMPLIB_RELEASE ${ZSTD_LIB})
set_property(TARGET zstd PROPERTY IMPORTED_LOCATION_RELEASE ${ZSTD_DLL})
set_property(TARGET zstd PROPERTY IMPORTED_IMPLIB_DEBUG ${ZSTD_DEBUG_LIB})
set_property(TARGET zstd PROPERTY IMPORTED_LOCATION_DEBUG ${ZSTD_DEBUG_DLL})

macro(link_zstd name)
    target_link_libraries(${name} zstd)
    add_custom_command(
            TARGET ${name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:zstd> $<TARGET_FILE_DIR:${name}>
    )
endmacro()

macro(include_zstd name)
    target_include_directories(${name} PRIVATE ${ZSTD_INCLUDE})
endmacro()

macro(prepare_zstd name)
    include_zstd(${name})
    link_zstd(${name})
endmacro()
