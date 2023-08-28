cmake_minimum_required(VERSION 3.22)
project(resources_manager)
set(CMAKE_CXX_STANDARD 20)

option(STATIC_LIBRARY "Build manager as static library" YES)
option(COPY_LINKER_FILES "Copy linker files to targets output directory" NO)

if (ANDROID)
    set(RM_ARCH_PREFIX android)
elseif (WIN32)
    set(RM_ARCH_PREFIX win)
else ()
    message(FATAL_ERROR "Unsupported platform. Only android and windows supported")
endif ()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(RM_ARCH ${RM_ARCH_PREFIX}64)
else()
    set(RM_ARCH ${RM_ARCH_PREFIX}32)
endif()

if (MSVC)
    # Enable exceptions
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
endif()

include_directories(.)

add_subdirectory(third-party)
add_subdirectory(library)
if (NOT ANDROID)
    add_subdirectory(executable)
endif ()
