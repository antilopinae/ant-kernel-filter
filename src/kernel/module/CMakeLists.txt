cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

include(ExternalProject)
include(AntGlobalDefinitions REQUIRED)

# Compiler options
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    include(ClangOptions REQUIRED)
    include(ClangDefinitions REQUIRED)
else()
    message(FATAL_ERROR "Unsupported compiler toolchain")
endif()

include(PlatformInfo REQUIRED)
setup_platform_info()

# Set up the kernel headers
#if (PLATFORM_ID STREQUAL "Ubuntu")
#    # TODO: automatically download the kernel headers
#    include("cmake/kernel-headers/ubuntu.cmake")
#else()
#    message(FATAL_ERROR "Project error: unsupported platform (check CMakeLists.txt).")
#endif()
#include_directories(SYSTEM ${KERNEL_INCLUDE_DIRECTORIES})

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

set(MODULE_OUT ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}.ko)

set(MODULE_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/kernel-module.c
        ${CMAKE_CURRENT_SOURCE_DIR}/utils.c
        ${CMAKE_CURRENT_SOURCE_DIR}/greeter.cpp
)

set(MODULE_OBJ_FILES "")

foreach (SRC_FILE IN LISTS MODULE_SRC)
    get_filename_component(BASE_NAME ${SRC_FILE} NAME_WE)
    list(APPEND MODULE_OBJ_FILES "${BASE_NAME}.o")
endforeach ()

string(REPLACE ";" " " MODULE_OBJ_FILES_STR "${MODULE_OBJ_FILES}")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/Makefile" "
obj-m := ${MODULE_NAME}.o
${MODULE_NAME}-objs := ${MODULE_OBJ_FILES_STR}
# ${MODULE_NAME}-y := ${MODULE_OBJ_FILES_STR}

EXTRA_CFLAGS += -I${CMAKE_CURRENT_BINARY_DIR}/include
EXTRA_CXXFLAGS += -I${CMAKE_CURRENT_BINARY_DIR}/include

")

set(C_MODULE_OBJ_FILES "")
set(CPP_MODULE_OBJ_FILES "")

foreach (SRC_FILE IN LISTS MODULE_SRC)
    get_filename_component(FILE_EXT ${SRC_FILE} EXT)
    get_filename_component(BASE_NAME ${SRC_FILE} NAME_WE)

    if ("${FILE_EXT}" STREQUAL ".c")
        list(APPEND C_MODULE_OBJ_FILES "${BASE_NAME}.o")
        file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/Makefile"
                "${BASE_NAME}.o: CFLAGS_${BASE_NAME}.o := \$(EXTRA_CFLAGS)\n"
        )

    elseif ("${FILE_EXT}" STREQUAL ".cpp")
        list(APPEND CPP_MODULE_OBJ_FILES "${BASE_NAME}.o")
        file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/Makefile"
                "${BASE_NAME}.o: ${BASE_NAME}.cpp\n"
                "\t\$(CXX) -std=gnu++11 -fno-exceptions -fno-rtti \$(EXTRA_CXXFLAGS) -fPIC -c ${SRC_FILE} -o ${BASE_NAME}.o"
        )
    endif ()
endforeach ()

file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/Makefile" "\n
KDIR := ${KERNEL_SRC_DIR}
PWD := \$(shell pwd)

all:
\t\$(MAKE) -C \$(KDIR) M=\$(PWD) modules

clean:
\t\$(MAKE) -C \$(KDIR) M=\$(PWD) clean
")

add_custom_target(kernel-module-build
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MODULE_SRC} ${CMAKE_CURRENT_BINARY_DIR}/
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/ant-kernel
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/include/ant-kernel/ ${CMAKE_CURRENT_BINARY_DIR}/include/ant-kernel/
        COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_BINARY_DIR} make -f Makefile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        BYPRODUCTS ${MODULE_OUT}
)

if (NOT USE_LOCAL_KERNEL_BUILD_DIR)
    add_dependencies(kernel-module-build kernel-download-src)
endif ()

add_custom_target(kernel-module-install ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/modules
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MODULE_OUT} ${CMAKE_BINARY_DIR}/modules/
        DEPENDS kernel-module-build
        COMMENT "Copying ${MODULE_OUT} to ${CMAKE_BINARY_DIR}/modules/"
)

add_custom_target(kernel-module-clean
        COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_BINARY_DIR} make -f Makefile clean
)

message(STATUS "Kernel source/headers dir used: ${KERNEL_SRC_DIR}")
message(STATUS "Built module output expected at: ${MODULE_OUT}")

include(AddKernelModule)
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "*.c" "*.cpp")
add_kernel_module(kernel-module "${SOURCE_FILES}")

