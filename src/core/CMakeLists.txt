cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

add_library(${PROJECT_NAME}-core STATIC
        delete_me.cpp
)

target_include_directories(${PROJECT_NAME}-core PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_include_directories(${PROJECT_NAME}-core PRIVATE
        ${CMAKE_SOURCE_DIR}/src/core
)

if (UNIX)
    target_link_options(${PROJECT_NAME}-core PUBLIC -static)
    target_link_options(${PROJECT_NAME}-core PUBLIC -static-libgcc -static-libstdc++)
endif ()

set(nlohmann-json_IMPLICIT_CONVERSIONS OFF)

target_link_libraries(${PROJECT_NAME}-core PUBLIC
        Boost::system
        Boost::asio
        fmt::fmt-header-only
        argparse::argparse
        nlohmann_json::nlohmann_json
)

include(AntDefines REQUIRED)
add_ant_global_definitions(${PROJECT_NAME}-core)
