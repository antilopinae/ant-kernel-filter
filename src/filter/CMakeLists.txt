cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

include(AntGlobalDefinitions REQUIRED)

add_library(${FILTER_TARGET_NAME} STATIC
        block.cpp
        code-generator.cpp
        filter.cpp
        graph.cpp
        xml-parser.cpp
        lib/tinyxml/tinyxml.cpp
)

target_include_directories(${FILTER_TARGET_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}
)

target_include_directories(${FILTER_TARGET_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

if (UNIX)
    target_link_options(${FILTER_TARGET_NAME} PUBLIC -static)
    target_link_options(${FILTER_TARGET_NAME} PUBLIC -static-libgcc -static-libstdc++)
endif ()

target_link_libraries(${FILTER_TARGET_NAME} PUBLIC
        ${CORE_TARGET_NAME}
)

target_add_ant_global_definitions(${FILTER_TARGET_NAME})