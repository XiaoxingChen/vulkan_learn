add_library(framework
    pipeline_resource.cpp
    pipelines/pipeline_unary_operation.cpp)

target_link_libraries(framework utils)
target_include_directories(framework PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${PROJECT_SOURCE_DIR})