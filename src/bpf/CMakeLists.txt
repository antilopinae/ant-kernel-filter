cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

include(CheckToolsetBpf REQUIRED)
include(AntGlobalDefinitions REQUIRED)

if (NOT SHELL OR NOT CLANG OR NOT BPFTOOL)
    message(FATAL_ERROR
            "\n"
            "Please specify toolset in cmake/CheckToolsetBpf.cmake.\n"
            "You can specify it automatically with CLANG_EXECUTABLE, BPFTOOL_EXECUTABLE, SHELL_EXECUTABLE variables\n"
    )
endif ()

# eBPF resources
set(BPF_NAME "ant-bpf")
set(BPF_TARGET_NAME ${BPF_NAME})

set(BPF_BINARY_DIR ${BPF_NAME}-dir)
set(BPF_BINARY_PATH ${CMAKE_CURRENT_BINARY_DIR}/${BPF_BINARY_DIR})

set(BPF_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${BPF_NAME}.bpf.c")
set(BPF_OBJ_PATH "${BPF_BINARY_PATH}/${BPF_NAME}.bpf.o")

set(BPF_SKEL ${BPF_NAME}.skel.h)
set(BPF_SKEL_PATH "${BPF_BINARY_PATH}/${BPF_SKEL}")
set(VMLINUX_HEADER_PATH "${BPF_BINARY_PATH}/vmlinux.h")

# generate vmlinux.h
add_custom_command(
        OUTPUT ${VMLINUX_HEADER_PATH}
        COMMAND ${SHELL} -c "${BPFTOOL} btf dump file /sys/kernel/btf/vmlinux format c > ${VMLINUX_HEADER_PATH}"
        COMMENT "Generating vmlinux.h from system BTF"
        VERBATIM
)

# compile eBPF C file to .o
add_custom_command(
        OUTPUT ${BPF_OBJ_PATH}
        COMMAND ${SHELL} -c "${CLANG} -g -O2 -Wall -Wextra -Werror -target bpf -I${BPF_BINARY_PATH} -I${libbpf_SOURCE_DIR}/src -D__BPF__ -c ${BPF_SRC_PATH} -o ${BPF_OBJ_PATH}"
        DEPENDS ${VMLINUX_HEADER_PATH}
        COMMENT "Compiling BPF C code"
        VERBATIM
)

# generate eBPF skeleton header
add_custom_command(
        OUTPUT ${BPF_SKEL_PATH}
        COMMAND ${SHELL} -c "${BPFTOOL} gen skeleton ${BPF_OBJ_PATH} > ${BPF_SKEL_PATH}"
        DEPENDS ${BPF_OBJ_PATH}
        COMMENT "Generating BPF skeleton header"
        VERBATIM
)

add_custom_target(generate_and_compile_bpf ALL DEPENDS ${BPF_SKEL_PATH})

# main executable
add_executable(${BPF_TARGET_NAME} main.cpp)

target_include_directories(${BPF_TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${BPF_BINARY_PATH}
)

target_compile_definitions(${BPF_TARGET_NAME} PRIVATE BPF_SKEL_HEADER="${BPF_SKEL}")

target_link_options(${BPF_TARGET_NAME} PRIVATE -static-libgcc -static-libstdc++ -static)

target_link_libraries(${BPF_TARGET_NAME} PRIVATE
        nlohmann_json::nlohmann_json
        PkgConfig::LIBBPF
        PkgConfig::ZLIB
)

add_dependencies(${BPF_TARGET_NAME} generate_and_compile_bpf)

target_add_ant_global_definitions(${BPF_TARGET_NAME})