cmake_minimum_required(VERSION 3.2)

get_filename_component(TARGET_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
add_executable(${TARGET_NAME} ${TARGET_NAME}.cpp sample_utils.cpp)

target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(${TARGET_NAME} utils glfw )
target_compile_definitions(${TARGET_NAME} PUBLIC VK_NO_PROTOTYPES)
