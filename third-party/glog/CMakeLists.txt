set(GLOG_LIB ${CMAKE_CURRENT_SOURCE_DIR}/${RM_ARCH}/Release/glog.lib CACHE INTERNAL "")
set(GLOG_DLL ${CMAKE_CURRENT_SOURCE_DIR}/${RM_ARCH}/Release/glog.dll CACHE INTERNAL "")

set(GLOG_DEBUG_LIB ${CMAKE_CURRENT_SOURCE_DIR}/${RM_ARCH}/Debug/glogd.lib CACHE INTERNAL "")
set(GLOG_DEBUG_DLL ${CMAKE_CURRENT_SOURCE_DIR}/${RM_ARCH}/Debug/glogd.dll CACHE INTERNAL "")

set(GLOG_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "")

set(GLOG_STATIC_LIB ${CMAKE_CURRENT_SOURCE_DIR}/${RM_ARCH}/Release/glog_static.lib CACHE INTERNAL "")
set(GLOG_STATIC_DEBUG_LIB ${CMAKE_CURRENT_SOURCE_DIR}/${RM_ARCH}/Debug/glogd_static.lib CACHE INTERNAL "")

set(GLOG_BUILD_STATIC YES CACHE INTERNAL "")
if (NOT ${GLOG_BUILD_STATIC})
    add_library(glog SHARED IMPORTED GLOBAL)
    set_property(TARGET glog PROPERTY IMPORTED_IMPLIB_RELEASE ${GLOG_LIB})
    set_property(TARGET glog PROPERTY IMPORTED_LOCATION_RELEASE ${GLOG_DLL})
    set_property(TARGET glog PROPERTY IMPORTED_IMPLIB_DEBUG ${GLOG_DEBUG_LIB})
    set_property(TARGET glog PROPERTY IMPORTED_LOCATION_DEBUG ${GLOG_DEBUG_DLL})
endif()

macro(link_glog name)
    if (NOT ${GLOG_BUILD_STATIC})
        target_link_libraries(${name} glog)
        add_custom_command(
                TARGET ${name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:glog> $<TARGET_FILE_DIR:${name}>
        )
    else()
        target_link_libraries(${name} $<IF:$<CONFIG:Debug>,${GLOG_STATIC_DEBUG_LIB},${GLOG_STATIC_LIB}>)
    endif ()
endmacro()

macro(include_glog name)
    target_include_directories(${name} PRIVATE ${GLOG_INCLUDE})
endmacro()

macro(prepare_glog name)
    include_glog(${name})
    link_glog(${name})
endmacro()
