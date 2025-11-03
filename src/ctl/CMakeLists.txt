cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

# main executable
file(GLOB_RECURSE SOURCES "*.cpp")

foreach(SOURCE_FILE IN LISTS SOURCES)
    get_filename_component(SRC_NAME ${SOURCE_FILE} NAME_WE)

    set(EXEC_NAME "${PROJECT_NAME}-${SRC_NAME}")
    add_executable(${EXEC_NAME} ${SOURCE_FILE})

    target_include_directories(${EXEC_NAME} PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}
    )

    target_link_options(${EXEC_NAME} PRIVATE -static-libgcc -static-libstdc++ -static)

    target_link_libraries(${EXEC_NAME} PRIVATE
            Boost::asio
    )
endforeach()