cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

add_library(${CORE_LOG_TARGET_NAME} STATIC
        logger.cpp
)

target_include_directories(${CORE_LOG_TARGET_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}
)

target_include_directories(${CORE_LOG_TARGET_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

if (UNIX)
    target_link_options(${CORE_LOG_TARGET_NAME} PUBLIC -static)
    target_link_options(${CORE_LOG_TARGET_NAME} PUBLIC -static-libgcc -static-libstdc++)
endif ()

target_link_libraries(${CORE_LOG_TARGET_NAME} PUBLIC
        ${CORE_TARGET_NAME}
)

target_add_ant_global_definitions(${CORE_LOG_TARGET_NAME})
