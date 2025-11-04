cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

include(ExternalProject)
include(AntGlobalDefinitions REQUIRED)

execute_process(COMMAND uname -r
        OUTPUT_VARIABLE ANT_UNAME_R
        OUTPUT_STRIP_TRAILING_WHITESPACE)

message(STATUS "Detected kernel release: ${ANT_UNAME_R}")

set(LOCAL_KERNEL_BUILD_DIR "/lib/modules/${ANT_UNAME_R}/build")
set(USE_LOCAL_KERNEL_BUILD_DIR FALSE)

if (EXISTS "${LOCAL_KERNEL_BUILD_DIR}")
    message(STATUS "Found kernel build dir: ${LOCAL_KERNEL_BUILD_DIR} — will use system headers")
    set(USE_LOCAL_KERNEL_BUILD_DIR TRUE)
endif ()

if (NOT USE_LOCAL_KERNEL_BUILD_DIR)
    string(REGEX MATCH "([0-9]+\\.[0-9]+\\.[0-9]+)" _ MATCHED ${ANT_UNAME_R})
    if (MATCHED)
        set(ANT_KERNEL_VERSION ${CMAKE_MATCH_1})
        message(STATUS "Extracted semantic kernel version: ${ANT_KERNEL_VERSION}")
    else ()
        message(FATAL_ERROR "Cannot extract semantic X.Y.Z from ${ANT_UNAME_R}; set kernel version manually")
    endif ()

    set(ANT_KERNEL_TARBALL "linux-${ANT_KERNEL_VERSION}.tar.xz")
    set(ANT_KERNEL_URL "https://cdn.kernel.org/pub/linux/kernel/v6.x/${ANT_KERNEL_TARBALL}")

    ExternalProject_Add(kernel-download-src
            URL ${ANT_KERNEL_URL}
            PREFIX ${CMAKE_BINARY_DIR}/kernel-src
            CONFIGURE_COMMAND ""
            BUILD_COMMAND make -C <SOURCE_DIR> defconfig && make -C <SOURCE_DIR> vmlinux -j${CMAKE_BUILD_PARALLEL_LEVEL}
            BUILD_BYPRODUCTS <SOURCE_DIR>/vmlinux
            INSTALL_COMMAND ""
    )

    ExternalProject_Get_Property(kernel-download-src SOURCE_DIR)
    set(KERNEL_SRC_DIR ${SOURCE_DIR})

    add_custom_target(kernel-headers ALL
            DEPENDS ${SOURCE_DIR}/vmlinux
            COMMENT "Kernel vmlinux will be available at: ${SOURCE_DIR}"
    )

    add_dependencies(kernel-headers kernel-download-src)
else ()
    set(KERNEL_SRC_DIR ${LOCAL_KERNEL_BUILD_DIR})
    add_custom_target(kernel-headers ALL
            COMMENT "Using system kernel headers at ${KERNEL_SRC_DIR}"
    )
endif ()

if (USE_LOCAL_KERNEL_BUILD_DIR)
    if (NOT EXISTS "${KERNEL_SRC_DIR}/Makefile")
        message(FATAL_ERROR "Kernel build dir ${KERNEL_SRC_DIR} has no Makefile. Install linux-headers for your kernel.")
    endif ()
endif ()

set(MODULE_NAME kernel_module)
set(MODULE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_NAME}.c)
set(MODULE_OUT ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}.ko)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/Makefile" "
obj-m := ${MODULE_NAME}.o
EXTRA_CFLAGS += -I${CMAKE_CURRENT_SOURCE_DIR}/include

KDIR := ${KERNEL_SRC_DIR}
PWD := \$(shell pwd)

all:
\t\$(MAKE) -C \$(KDIR) M=\$(PWD) modules

clean:
\t\$(MAKE) -C \$(KDIR) M=\$(PWD) clean
")

add_custom_target(kernel_module_build
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${MODULE_SRC}
        ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}.c
        COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_BINARY_DIR} make -f Makefile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        BYPRODUCTS ${MODULE_OUT}
)

if (NOT USE_LOCAL_KERNEL_BUILD_DIR)
    add_dependencies(kernel_module_build kernel-download-src)
endif ()

add_custom_target(kernel_module_install ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/modules
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MODULE_OUT} ${CMAKE_BINARY_DIR}/modules/
        DEPENDS kernel_module_build
        COMMENT "Copying ${MODULE_OUT} to ${CMAKE_BINARY_DIR}/modules/"
)

add_custom_target(kernel_module_clean
        COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_BINARY_DIR} make -f Makefile clean
)

message(STATUS "Kernel source/headers dir used: ${KERNEL_SRC_DIR}")
message(STATUS "Built module output expected at: ${MODULE_OUT}")

add_executable(${KERNEL_TARGET_NAME}
        main.cpp
)

target_include_directories(${KERNEL_TARGET_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}
)

target_include_directories(${KERNEL_TARGET_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (UNIX)
    target_link_options(${FILTER_TARGET_NAME} PUBLIC -static)
    target_link_options(${FILTER_TARGET_NAME} PUBLIC -static-libgcc -static-libstdc++)
endif ()

target_link_libraries(${KERNEL_TARGET_NAME} PUBLIC
        ${CORE_TARGET_NAME}
)

add_dependencies(${KERNEL_TARGET_NAME} kernel_module_install)

target_add_ant_global_definitions(${KERNEL_TARGET_NAME})
