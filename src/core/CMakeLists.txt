cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

include(AntGlobalDefinitions REQUIRED)

add_library(${CORE_TARGET_NAME} STATIC
        error.cpp
)

target_include_directories(${CORE_TARGET_NAME} PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_include_directories(${CORE_TARGET_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/core
)

if (UNIX)
    target_link_options(${CORE_TARGET_NAME} PUBLIC -static)
    target_link_options(${CORE_TARGET_NAME} PUBLIC -static-libgcc -static-libstdc++)
endif ()

set(nlohmann-json_IMPLICIT_CONVERSIONS OFF)

target_link_libraries(${CORE_TARGET_NAME} PUBLIC
        Boost::system
        Boost::asio
        fmt::fmt-header-only
        argparse::argparse
        nlohmann_json::nlohmann_json
        pthread
)

target_add_ant_global_definitions(${CORE_TARGET_NAME})
