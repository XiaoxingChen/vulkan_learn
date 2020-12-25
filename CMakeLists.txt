cmake_minimum_required(VERSION 3.1)
project(vulkan_learn)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(WriteShaderBank)

add_subdirectory(third_party)
add_subdirectory(src)
add_subdirectory(utils)
if(BUILD_TEST)
    add_subdirectory(tests)
endif(BUILD_TEST)
