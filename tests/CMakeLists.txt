cmake_minimum_required(VERSION 3.2)

set(SHADER_LIST basic.comp basic2.comp)
WriteShaderBank(${CMAKE_CURRENT_SOURCE_DIR}/shaders "${SHADER_LIST}" ${CMAKE_CURRENT_SOURCE_DIR}/../utils/shader_bank.h.in ${CMAKE_CURRENT_BINARY_DIR}/shader_bank.h)
add_executable(test_main test_main.cpp)
target_include_directories(test_main PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(test_main utils)
target_compile_definitions(test_main PUBLIC VK_NO_PROTOTYPES NDEBUG)
