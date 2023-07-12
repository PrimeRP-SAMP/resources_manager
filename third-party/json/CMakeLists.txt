set(JSON_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "")

macro(include_json name)
    target_include_directories(${name} PRIVATE ${JSON_INCLUDE})
endmacro()

macro(prepare_json name)
    include_json(${name})
endmacro()
