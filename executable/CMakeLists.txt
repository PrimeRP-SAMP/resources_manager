set(EXEC_NAME ${PROJECT_NAME}_preparer)

add_executable(${EXEC_NAME} main.cpp)
prepare_zstd(${EXEC_NAME})
prepare_json(${EXEC_NAME})
