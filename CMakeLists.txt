cmake_minimum_required(VERSION 3.1)
project(vulkan_learn)

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
  set(MACOSX TRUE)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
  set(WINDOWS TRUE)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(LINUX TRUE)
endif()

# find_package(glm REQUIRED)

if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(textualResourceManager)

add_subdirectory(third_party)
add_subdirectory(utils)
if(BUILD_TEST)
    add_subdirectory(tests)
endif(BUILD_TEST)

set(STATIC_RESOURCE_MANAGER_TEMPLATE ${PROJECT_SOURCE_DIR}/utils/resource_manager_static.cpp.in)
set(BINARIZE_RESOURCE_MANAGER_TEMPLATE ${PROJECT_SOURCE_DIR}/utils/resource_manager_binarize.cpp.in)
set(ASSETS_FOLDER_STR ${PROJECT_SOURCE_DIR}/build/assets/)

# add_subdirectory(create_device)
add_subdirectory(simple_compute)
# add_subdirectory(vector_crash_android)
add_subdirectory(hello_triangle)
add_subdirectory(model_loading)
add_subdirectory(image_processing_toys)
