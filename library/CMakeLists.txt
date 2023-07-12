set(LIB_NAME ${PROJECT_NAME}_library)

add_library(${LIB_NAME} SHARED main.cpp rm_tree.cpp rm_entry.cpp rm_cdn.cpp resources_manager.cpp)
prepare_curl(${LIB_NAME})
prepare_zstd(${LIB_NAME})
prepare_json(${LIB_NAME})
prepare_glog(${LIB_NAME})

target_precompile_headers(${LIB_NAME} PRIVATE pch.h)
target_compile_definitions(${LIB_NAME} PRIVATE RM_BUILDING_DLL)
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") # Pure MSVC compiler only
    target_compile_options(${LIB_NAME} PRIVATE "/Zc:preprocessor")
endif ()
