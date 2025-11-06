cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

include(ExternalProject)

include(AddCKernelModule REQUIRED)

set(ANT_MODULE_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/kernel-module.c
        ${CMAKE_CURRENT_SOURCE_DIR}/utils.c
)

add_c_kernel_module(ant-kernel-module ${CMAKE_CURRENT_SOURCE_DIR}
        ${ANT_MODULE_SRC}
)

