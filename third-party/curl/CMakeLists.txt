option(USE_CURL_STATIC "Bundle curl as static library" YES)

set(LIBCURL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "")
set(LIBCURL_CERT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/curl-ca-bundle.crt CACHE INTERNAL "")

if (WIN32)
    set(LIBCURL_STATIC_LIB_FILENAME libcurl_static.lib CACHE INTERNAL "")
    set(LIBCURL_STATIC_DEBUG_LIB_FILENAME libcurl-d_static.lib CACHE INTERNAL "")

    set(LIBCURL_SHARED_LIB_FILENAME libcurl_imp.lib CACHE INTERNAL "")
    set(LIBCURL_SHARED_DEBUG_LIB_FILENAME libcurl-d_imp.lib CACHE INTERNAL "")

    set(LIBCURL_SHARED_IMPLIB_FILENAME libcurl.dll CACHE INTERNAL "")
    set(LIBCURL_SHARED_DEBUG_IMPLIB_FILENAME libcurl-d.dll CACHE INTERNAL "")

    set(LIBCURL_CERT_FILENAME curl-ca-bundle.crt CACHE INTERNAL "")
elseif (ANDROID)
    set(LIBCURL_STATIC_LIB_FILENAME libcurl_static.a CACHE INTERNAL "")
    set(LIBCURL_STATIC_DEBUG_LIB_FILENAME libcurl-d_static.a CACHE INTERNAL "")

    set(LIBCURL_SHARED_IMPLIB_FILENAME libcurl.so CACHE INTERNAL "")
    set(LIBCURL_SHARED_DEBUG_IMPLIB_FILENAME libcurl-d.so CACHE INTERNAL "")

    set(LIBCURL_CERT_FILENAME ca-bundle.crt CACHE INTERNAL "")
endif ()

set(LIBCURL_LIBS_DEBUG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${RM_ARCH}/Debug/)
set(LIBCURL_LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${RM_ARCH}/Release/)

if (USE_CURL_STATIC)
    add_library(libcurl STATIC IMPORTED GLOBAL)
    set_property(TARGET libcurl PROPERTY IMPORTED_LOCATION_RELEASE ${LIBCURL_LIBS_DIR}/${LIBCURL_STATIC_LIB_FILENAME})
    set_property(TARGET libcurl PROPERTY IMPORTED_LOCATION_DEBUG ${LIBCURL_LIBS_DEBUG_DIR}/${LIBCURL_STATIC_DEBUG_LIB_FILENAME})
else ()
    add_library(libcurl SHARED IMPORTED GLOBAL)
    set_property(TARGET libcurl PROPERTY IMPORTED_LOCATION_RELEASE ${LIBCURL_LIBS_DIR}/${LIBCURL_SHARED_IMPLIB_FILENAME})
    set_property(TARGET libcurl PROPERTY IMPORTED_LOCATION_DEBUG ${LIBCURL_LIBS_DEBUG_DIR}/${LIBCURL_SHARED_DEBUG_IMPLIB_FILENAME})
    if (WIN32)
        set_property(TARGET libcurl PROPERTY IMPORTED_IMPLIB_RELEASE ${LIBCURL_LIBS_DIR}/${LIBCURL_SHARED_LIB_FILENAME})
        set_property(TARGET libcurl PROPERTY IMPORTED_IMPLIB_DEBUG ${LIBCURL_LIBS_DEBUG_DIR}/${LIBCURL_SHARED_DEBUG_LIB_FILENAME})
    endif ()
endif ()

macro(link_curl name)
    target_link_libraries(${name} libcurl)
    if (NOT USE_CURL_STATIC)
        add_custom_command(
                TARGET ${name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:libcurl> $<TARGET_FILE_DIR:${name}>
        )
    endif ()
    if (COPY_LINKER_FILES)
        add_custom_command(
                TARGET ${name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_LINKER_FILE:libcurl> $<TARGET_FILE_DIR:${name}>
        )
    endif ()
endmacro()

macro(include_curl name)
    if (USE_CURL_STATIC)
        target_compile_definitions(${name} PRIVATE CURL_STATICLIB)
    endif ()
    target_include_directories(${name} PRIVATE ${LIBCURL_INCLUDE})
endmacro()

macro(prepare_curl name)
    include_curl(${name})
    link_curl(${name})
    add_custom_command(
            TARGET ${name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${LIBCURL_CERT_FILE} $<TARGET_FILE_DIR:${name}>/${LIBCURL_CERT_FILENAME}
    )
endmacro()
