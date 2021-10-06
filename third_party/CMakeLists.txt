cmake_minimum_required(VERSION 3.2)

add_library(vulkan INTERFACE)
set(VULKAN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/include)
target_include_directories(vulkan INTERFACE ${VULKAN_INCLUDE_DIR})

add_subdirectory(glslang)
add_subdirectory(spdlog)
if(NOT ANDROID)
    add_subdirectory(glfw)
endif(NOT ANDROID)
add_subdirectory(tinygltf)
add_subdirectory(glm)
