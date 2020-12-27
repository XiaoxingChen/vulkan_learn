cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME vector_crash_android)

add_executable(${TARGET_NAME} ${TARGET_NAME}.cpp)
target_link_libraries(${TARGET_NAME} utils)
target_compile_definitions(${TARGET_NAME} PUBLIC VK_NO_PROTOTYPES NDEBUG)
