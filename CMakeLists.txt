cmake_minimum_required(VERSION 3.22)
project(resources_manager)
set(CMAKE_CXX_STANDARD 20)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(RM_ARCH win64)
else()
    set(RM_ARCH win32)
endif()

if (MSVC)
    # Enable exceptions
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
endif()

include_directories(.)

add_subdirectory(third-party)
add_subdirectory(library)
add_subdirectory(executable)
