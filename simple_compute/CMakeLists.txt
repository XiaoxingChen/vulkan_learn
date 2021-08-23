cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME simple_compute)
add_executable(${TARGET_NAME} main.cpp)

set(SHADER_LIST test.comp)
WriteShaderBank(${CMAKE_CURRENT_SOURCE_DIR}/shaders "${SHADER_LIST}" ${CMAKE_CURRENT_SOURCE_DIR}/../utils/shader_bank.h.in ${CMAKE_CURRENT_BINARY_DIR}/shader_bank.h)
target_link_libraries(${TARGET_NAME} utils)
target_compile_definitions(${TARGET_NAME} PUBLIC VK_NO_PROTOTYPES NDEBUG)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
