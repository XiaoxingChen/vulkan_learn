cmake_minimum_required(VERSION 3.2)

add_executable(main main.cpp)

set(SHADER_LIST test.comp)
WriteShaderBank(${CMAKE_CURRENT_SOURCE_DIR}/shaders "${SHADER_LIST}" ${CMAKE_CURRENT_SOURCE_DIR}/../utils/shader_bank.h.in ${CMAKE_CURRENT_BINARY_DIR}/shader_bank.h)
target_link_libraries(main utils)
target_compile_definitions(main PUBLIC VK_NO_PROTOTYPES NDEBUG)
target_include_directories(main PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
