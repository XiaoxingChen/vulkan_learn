cmake_minimum_required(VERSION 3.2)

get_filename_component(TARGET_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

set(SHADER_LIST test.comp)
# WriteShaderBank(${CMAKE_CURRENT_SOURCE_DIR}/shaders "${SHADER_LIST}" ${CMAKE_CURRENT_SOURCE_DIR}/../utils/shader_bank.h.in ${CMAKE_CURRENT_BINARY_DIR}/shader_bank.h)
set(SHADER_FOLDER_STR ${CMAKE_CURRENT_SOURCE_DIR}/shaders/)
serializeToByteString(
    ${SHADER_FOLDER_STR}
    "${SHADER_LIST}"
    SHADER_HEX_STRING
    SHADER_SRC_MAP_STRING)

# message(${SHADER_HEX_STRING})

# set(RESOURCE_TEMPLATE_FILE ${PROJECT_SOURCE_DIR}/utils/resource_manager_binarize.cpp.in)
set(RESOURCE_TEMPLATE_FILE ${PROJECT_SOURCE_DIR}/utils/resource_manager_static.cpp.in)

configure_file (
        ${RESOURCE_TEMPLATE_FILE}
        ${CMAKE_CURRENT_BINARY_DIR}/resource_manager.cpp)

add_executable(${TARGET_NAME}
    ${TARGET_NAME}.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/resource_manager.cpp)
target_link_libraries(${TARGET_NAME} utils)
target_compile_definitions(${TARGET_NAME} PUBLIC VK_NO_PROTOTYPES)
if(NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
    target_compile_definitions(${TARGET_NAME} PUBLIC NDEBUG)
endif()
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
