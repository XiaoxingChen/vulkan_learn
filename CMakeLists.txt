cmake_minimum_required(VERSION 3.1)
project(vulkan_learn)

if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(WriteShaderBank)

add_subdirectory(third_party)
add_subdirectory(utils)
if(BUILD_TEST)
    add_subdirectory(tests)
endif(BUILD_TEST)


# add_subdirectory(create_device)
# add_subdirectory(simple_compute)
# add_subdirectory(vector_crash_android)
add_subdirectory(hello_triangle)
