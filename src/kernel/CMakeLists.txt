cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

include(ExternalProject)

set(MODULE_NAME ant-kernel CACHE STRING "Kernel module name" FORCE)

add_subdirectory(module)

add_library(${KERNEL_TARGET_NAME} STATIC
        kernel-module.cpp
        mmap.cpp
)

target_include_directories(${KERNEL_TARGET_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/module/include
)

target_include_directories(${KERNEL_TARGET_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if (UNIX)
    target_link_options(${KERNEL_TARGET_NAME} PUBLIC -static)
    target_link_options(${KERNEL_TARGET_NAME} PUBLIC -static-libgcc -static-libstdc++)
endif ()

target_link_libraries(${KERNEL_TARGET_NAME} PUBLIC
        ${CORE_TARGET_NAME}
        ${CORE_LOG_TARGET_NAME}
)

# Building kernel module
add_dependencies(${KERNEL_TARGET_NAME} ant-kernel-module)

target_add_ant_global_definitions(${KERNEL_TARGET_NAME})
