cmake_minimum_required(VERSION 3.2)

get_filename_component(TARGET_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

set(SHADER_LIST imshow.vert)
set(SHADER_MANAGER_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/resource_manager.cpp)
generateShaderManagerCode(
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/
    "${SHADER_LIST}"
    ${SHADER_MANAGER_OUTPUT_PATH}
    TRUE)

add_executable(${TARGET_NAME}
    ${TARGET_NAME}.cpp
    sample_utils.cpp
    access_euroc_dataset.cpp
    ${SHADER_MANAGER_OUTPUT_PATH})

target_include_directories(${TARGET_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_BINARY_DIR})
target_link_libraries(${TARGET_NAME} utils glfw )
target_compile_definitions(${TARGET_NAME} PUBLIC VK_NO_PROTOTYPES)

if(CMAKE_BUILD_TYPE EQUAL "DEBUG")
    target_compile_definitions(${TARGET_NAME} PUBLIC NDEBUG)
endif()
