cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

include(AntGlobalDefinitions REQUIRED)

# main executable
add_executable(${PROJECT_NAME}
        main.cpp
        prometheus.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/ctl
)

if (UNIX)
    target_link_options(${PROJECT_NAME} PUBLIC -static)
    target_link_options(${PROJECT_NAME} PUBLIC -static-libgcc -static-libstdc++)
endif ()

target_link_libraries(${PROJECT_NAME} PUBLIC
        ${PROJECT_NAME}-core
        prometheus-cpp::core
        prometheus-cpp::pull
        prometheus-cpp::util
)

target_add_ant_global_definitions(${PROJECT_NAME})