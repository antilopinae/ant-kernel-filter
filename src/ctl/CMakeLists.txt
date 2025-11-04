cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

# main executable
add_executable(${PROJECT_NAME}
        main.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if (UNIX)
    target_link_options(${PROJECT_NAME} PUBLIC -static)
    target_link_options(${PROJECT_NAME} PUBLIC -static-libgcc -static-libstdc++)
endif ()

target_link_libraries(${PROJECT_NAME} PUBLIC
        Boost::system
        fmt::fmt-header-only
        argparse::argparse
)

include(AntDefines REQUIRED)
add_ant_global_definitions(${PROJECT_NAME})