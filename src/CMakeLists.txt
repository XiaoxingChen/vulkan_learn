cmake_minimum_required(VERSION 3.2)

add_executable(create_device create_device.cpp)

target_link_libraries(create_device utils)
target_compile_definitions(create_device PUBLIC VK_NO_PROTOTYPES NDEBUG)
