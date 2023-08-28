option(USE_ZSTD_STATIC "Bundle zstd as static library" YES)

set(ZSTD_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "")

if (WIN32)
    set(ZSTD_STATIC_LIB_FILENAME zstd_static.lib CACHE INTERNAL "")
    set(ZSTD_STATIC_DEBUG_LIB_FILENAME zstd_static.lib CACHE INTERNAL "")

    set(ZSTD_SHARED_LIB_FILENAME zstd.lib CACHE INTERNAL "")
    set(ZSTD_SHARED_DEBUG_LIB_FILENAME zstd.lib CACHE INTERNAL "")

    set(ZSTD_SHARED_IMPLIB_FILENAME zstd.dll CACHE INTERNAL "")
    set(ZSTD_SHARED_DEBUG_IMPLIB_FILENAME zstd.dll CACHE INTERNAL "")
elseif (ANDROID)
    set(ZSTD_STATIC_LIB_FILENAME libzstd.a CACHE INTERNAL "")
    set(ZSTD_STATIC_DEBUG_LIB_FILENAME libzstd.a CACHE INTERNAL "")

    set(ZSTD_SHARED_IMPLIB_FILENAME libzstd.so CACHE INTERNAL "")
    set(ZSTD_SHARED_DEBUG_IMPLIB_FILENAME libzstd.so CACHE INTERNAL "")
endif ()

set(ZSTD_LIBS_DEBUG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${RM_ARCH}/Debug/)
set(ZSTD_LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${RM_ARCH}/Release/)

if (USE_ZSTD_STATIC)
    add_library(zstd STATIC IMPORTED GLOBAL)
    set_property(TARGET zstd PROPERTY IMPORTED_LOCATION_RELEASE ${ZSTD_LIBS_DIR}/${ZSTD_STATIC_LIB_FILENAME})
    set_property(TARGET zstd PROPERTY IMPORTED_LOCATION_DEBUG ${ZSTD_LIBS_DEBUG_DIR}/${ZSTD_STATIC_DEBUG_LIB_FILENAME})
else ()
    add_library(zstd SHARED IMPORTED GLOBAL)
    set_property(TARGET zstd PROPERTY IMPORTED_LOCATION_RELEASE ${ZSTD_LIBS_DIR}/${ZSTD_SHARED_IMPLIB_FILENAME})
    set_property(TARGET zstd PROPERTY IMPORTED_LOCATION_DEBUG ${ZSTD_LIBS_DEBUG_DIR}/${ZSTD_SHARED_DEBUG_IMPLIB_FILENAME})
    if (WIN32)
        set_property(TARGET zstd PROPERTY IMPORTED_IMPLIB_RELEASE ${ZSTD_LIBS_DIR}/${ZSTD_SHARED_LIB_FILENAME})
        set_property(TARGET zstd PROPERTY IMPORTED_IMPLIB_DEBUG ${ZSTD_LIBS_DEBUG_DIR}/${ZSTD_SHARED_DEBUG_LIB_FILENAME})
    endif ()
endif ()

macro(link_zstd name)
    target_link_libraries(${name} zstd)
    if (NOT USE_ZSTD_STATIC)
        add_custom_command(
                TARGET ${name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:zstd> $<TARGET_FILE_DIR:${name}>
        )
    endif ()
    if (COPY_LINKER_FILES)
        add_custom_command(
                TARGET ${name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_LINKER_FILE:zstd> $<TARGET_FILE_DIR:${name}>
        )
    endif ()
endmacro()

macro(include_zstd name)
    target_include_directories(${name} PRIVATE ${ZSTD_INCLUDE})
endmacro()

macro(prepare_zstd name)
    include_zstd(${name})
    link_zstd(${name})
endmacro()

