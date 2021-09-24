# Copyright(c) 2020, NVIDIA CORPORATION. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.2)

project(utils)

set(HEADERS
  geometries.hpp
  math.hpp
  shaders.hpp
  utils.hpp
)

set(SOURCES
  math.cpp
  shaders.cpp
  utils.cpp
  gltf_utils.cpp
  event_manager.cpp
)

source_group(headers FILES ${HEADERS})
source_group(sources FILES ${SOURCES})

add_library(utils
  ${SOURCES}
  ${HEADERS}
)

target_link_libraries(utils PUBLIC vulkan)
if(NOT WINDOWS)
  target_link_libraries(utils PUBLIC dl)
endif()
target_link_libraries(utils PUBLIC spdlog)
if(ANDROID)
  target_link_libraries(utils PUBLIC android log)
endif()
if(MACOSX)
  target_link_libraries(utils PUBLIC glm::glm)
else()
  target_link_libraries(utils PUBLIC glm)
endif()

target_link_libraries(utils PUBLIC glfw)
target_link_libraries(utils PUBLIC glslang)
target_link_libraries(utils PUBLIC glslang-default-resource-limits)
target_link_libraries(utils PUBLIC SPIRV)
target_link_libraries(utils PUBLIC tinygltf)
target_compile_definitions(utils PUBLIC VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 VK_ENABLE_BETA_EXTENSIONS)
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# set(ASSETS_FOLDER ${PROJECT_BINARY_DIR}/../../assets/)
# configure_file(assets_manager.h.in ${CMAKE_CURRENT_BINARY_DIR}/assets_manager.h)

