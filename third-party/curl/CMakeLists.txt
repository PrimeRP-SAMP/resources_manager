set(LIBCURL_LIB ${CMAKE_CURRENT_SOURCE_DIR}/${RM_ARCH}/lib/libcurl.dll.lib CACHE INTERNAL "")

if (${RM_ARCH} STREQUAL "win32")
    set(LIBCURL_DLL ${CMAKE_CURRENT_SOURCE_DIR}/${RM_ARCH}/bin/libcurl.dll CACHE INTERNAL "")
else()
    set(LIBCURL_DLL ${CMAKE_CURRENT_SOURCE_DIR}/${RM_ARCH}/bin/libcurl-x64.dll CACHE INTERNAL "")
endif()

set(LIBCURL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/${RM_ARCH}/include CACHE INTERNAL "")

add_library(libcurl SHARED IMPORTED GLOBAL)
set_property(TARGET libcurl PROPERTY IMPORTED_IMPLIB_RELEASE ${LIBCURL_LIB})
set_property(TARGET libcurl PROPERTY IMPORTED_LOCATION_RELEASE ${LIBCURL_DLL})
set_property(TARGET libcurl PROPERTY IMPORTED_IMPLIB_DEBUG ${LIBCURL_LIB})
set_property(TARGET libcurl PROPERTY IMPORTED_LOCATION_DEBUG ${LIBCURL_DLL})

macro(link_curl name)
    target_link_libraries(${name} libcurl)
    add_custom_command(
            TARGET ${name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:libcurl> $<TARGET_FILE_DIR:${name}>
    )
endmacro()

macro(include_curl name)
    target_include_directories(${name} PRIVATE ${LIBCURL_INCLUDE})
endmacro()

macro(prepare_curl name)
    include_curl(${name})
    link_curl(${name})
endmacro()
