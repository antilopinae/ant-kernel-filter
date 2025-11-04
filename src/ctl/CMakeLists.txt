cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

include(AntGlobalDefinitions REQUIRED)

# main executable
add_executable(${CTL_TARGET_NAME}
        main.cpp
        prometheus.cpp
)

target_include_directories(${CTL_TARGET_NAME} PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_include_directories(${CTL_TARGET_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/ctl
)

if (UNIX)
    target_link_options(${CTL_TARGET_NAME} PUBLIC -static)
    target_link_options(${CTL_TARGET_NAME} PUBLIC -static-libgcc -static-libstdc++)
endif ()

target_link_libraries(${CTL_TARGET_NAME} PUBLIC
        ${CORE_TARGET_NAME}
        prometheus-cpp::core
        prometheus-cpp::pull
        prometheus-cpp::util
)

target_add_ant_global_definitions(${CTL_TARGET_NAME})