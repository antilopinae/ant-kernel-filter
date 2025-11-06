cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/compilers")

include(CCache OPTIONAL)
include(NoInSourceBuilds OPTIONAL)

option(ANT_USE_VCPKG_TOOLCHAIN "Use vcpkg toolchain file" ON)
option(ANT_ENABLE_KERNEL "Include Kernel program" ON)
option(ANT_ENABLE_EBPF "Include eBPF program" ON)
option(ANT_ENABLE_TESTING "Include tests" ON)

if (NOT UNIX)
    if (WIN32 AND MINGW)
        set(VCPKG_TARGET_TRIPLET "x64-mingw-static" CACHE STRING "vcpkg triplet" FORCE)
    else ()
        message(FATAL_ERROR
                "\n"
                "Not UNIX builds are not allowed.\n"
        )
    endif ()
endif ()

if (ANT_USE_VCPKG_TOOLCHAIN)
    set(VCPKG_OVERLAY_PORTS "${CMAKE_SOURCE_DIR}/vcpkg-ports")

    if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "Vcpkg toolchain file")
    endif ()
endif ()

# project setup
project(ant-kernel-filter VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

if (UNIX)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build only static libraries" FORCE)
endif ()

# dependencies
find_package(Boost REQUIRED COMPONENTS system asio)
find_package(fmt REQUIRED)
find_package(argparse CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(prometheus-cpp CONFIG REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBBPF REQUIRED IMPORTED_TARGET libbpf)
pkg_check_modules(ZLIB REQUIRED IMPORTED_TARGET zlib)

set(CORE_TARGET_NAME ant-core CACHE STRING "Ant-Core target name" FORCE)
set(CORE_LOG_TARGET_NAME ant-core-log CACHE STRING "Ant-Core-Log target name" FORCE)
set(CTL_TARGET_NAME ant-ctl CACHE STRING "Ant-Ctl target name" FORCE)
set(FILTER_TARGET_NAME ant-filter CACHE STRING "Ant-Filter module target name" FORCE)
set(KERNEL_TARGET_NAME ant-kernel CACHE STRING "Ant-Kernel module target name" FORCE)
set(BPF_TARGET_NAME ant-bpf CACHE STRING "Ant-Bpf module target name" FORCE)

function(setup_platform_info)
    # Locate lsb_release
    find_program(LSB_RELEASE_EXECUTABLE lsb_release)
    if (NOT LSB_RELEASE_EXECUTABLE)
        message(FATAL_ERROR "Failed to locate the lsb_release executable.")
    endif ()

    # Fetch information from lsb_release
    execute_process(
            COMMAND ${LSB_RELEASE_EXECUTABLE} --short --id
            OUTPUT_VARIABLE LSB_RELEASE_ID
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
            COMMAND ${LSB_RELEASE_EXECUTABLE} --short --release
            OUTPUT_VARIABLE LSB_RELEASE_RELEASE
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
            COMMAND ${LSB_RELEASE_EXECUTABLE} --short --codename
            OUTPUT_VARIABLE LSB_RELEASE_CODENAME
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Set the variables
    set(ANT_PLATFORM_ID "${LSB_RELEASE_ID}" CACHE INTERNAL "Ant-Platform Id")
    set(ANT_PLATFORM_RELEASE "${LSB_RELEASE_RELEASE}" PARENT_SCOPE CACHE INTERNAL "Ant-Platform Release")
    set(ANT_PLATFORM_CODENAME "${LSB_RELEASE_CODENAME}" PARENT_SCOPE CACHE INTERNAL "Ant-Platform Codename")
endfunction()

setup_platform_info()

set(ANT_GLOBAL_DEFINITIONS
        ANT_PROGRAM_NAME="ant-kernel-filter"
        ANT_PLATFORM_ID=${ANT_PLATFORM_ID}
        ANT_PLATFORM_RELEASE=${ANT_PLATFORM_RELEASE}
        ANT_PLATFORM_CODENAME=${ANT_PLATFORM_CODENAME}
        ANT_KERNEL_MODULE_ENABLE=${ANT_ENABLE_KERNEL}
        ANT_EBPF_MODULE_ENABLE=${ANT_ENABLE_EBPF}
        CACHE INTERNAL "Globally shared ant definitions"
)

function(target_add_ant_global_definitions TARGET_NAME)
    target_compile_definitions(${TARGET_NAME}
            PRIVATE
            ${ANT_GLOBAL_DEFINITIONS}
    )
endfunction()

add_subdirectory(src/core)
add_subdirectory(src/log)
add_subdirectory(src/ctl)
add_subdirectory(src/filter)

if (ANT_ENABLE_KERNEL)
    include(SetupKernelHeadersDir REQUIRED)
    add_subdirectory(src/kernel)
endif ()

if (ANT_ENABLE_EBPF)
    add_subdirectory(src/bpf)
endif ()

if (ANT_ENABLE_TESTING)
    add_subdirectory(src/test)
endif ()
