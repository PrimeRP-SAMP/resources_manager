option(USE_GLOG_STATIC "Bundle glog as static library" YES)

set(GLOG_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "")

if (WIN32)
    set(GLOG_STATIC_LIB_FILENAME glog_static.lib CACHE INTERNAL "")
    set(GLOG_STATIC_DEBUG_LIB_FILENAME glogd_static.lib CACHE INTERNAL "")

    set(GLOG_SHARED_LIB_FILENAME glog.lib CACHE INTERNAL "")
    set(GLOG_SHARED_DEBUG_LIB_FILENAME glogd.lib CACHE INTERNAL "")

    set(GLOG_SHARED_IMPLIB_FILENAME glog.dll CACHE INTERNAL "")
    set(GLOG_SHARED_DEBUG_IMPLIB_FILENAME glogd.dll CACHE INTERNAL "")
endif ()

set(GLOG_LIBS_DEBUG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${RM_ARCH}/Debug/)
set(GLOG_LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${RM_ARCH}/Release/)

if (USE_GLOG_STATIC)
    add_library(glog STATIC IMPORTED GLOBAL)
    set_property(TARGET glog PROPERTY IMPORTED_LOCATION_RELEASE ${GLOG_LIBS_DIR}/${GLOG_STATIC_LIB_FILENAME})
    set_property(TARGET glog PROPERTY IMPORTED_LOCATION_DEBUG ${GLOG_LIBS_DEBUG_DIR}/${GLOG_STATIC_DEBUG_LIB_FILENAME})
else ()
    add_library(glog SHARED IMPORTED GLOBAL)
    set_property(TARGET glog PROPERTY IMPORTED_IMPLIB_RELEASE ${GLOG_LIBS_DIR}/${GLOG_SHARED_LIB_FILENAME})
    set_property(TARGET glog PROPERTY IMPORTED_LOCATION_RELEASE ${GLOG_LIBS_DIR}/${GLOG_SHARED_IMPLIB_FILENAME})
    set_property(TARGET glog PROPERTY IMPORTED_IMPLIB_DEBUG ${GLOG_LIBS_DEBUG_DIR}/${GLOG_SHARED_DEBUG_LIB_FILENAME})
    set_property(TARGET glog PROPERTY IMPORTED_LOCATION_DEBUG ${GLOG_LIBS_DEBUG_DIR}/${GLOG_SHARED_DEBUG_IMPLIB_FILENAME})
endif ()

macro(link_glog name)
    target_link_libraries(${name} glog)
    if (NOT USE_GLOG_STATIC)
        add_custom_command(
                TARGET ${name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:glog> $<TARGET_FILE_DIR:${name}>
        )
    endif ()
    if (COPY_LINKER_FILES)
        add_custom_command(
                TARGET ${name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_LINKER_FILE:glog> $<TARGET_FILE_DIR:${name}>
        )
    endif ()
endmacro()

macro(include_glog name)
    if (USE_GLOG_STATIC)
        target_compile_definitions(${name} PRIVATE GLOG_STATIC_DEFINE)
    endif ()
    target_include_directories(${name} PRIVATE ${GLOG_INCLUDE})
endmacro()

macro(prepare_glog name)
    include_glog(${name})
    link_glog(${name})
endmacro()
