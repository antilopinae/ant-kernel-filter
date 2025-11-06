set(KERNEL_HEADERS_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/kernel-headers")

# Copy the required header files
execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${KERNEL_BUILD_DIRECTORY}/include
        ${KERNEL_HEADERS_DIRECTORY}/include
)
execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${KERNEL_BUILD_DIRECTORY}/arch/${PROCESSOR_ARCHITECTURE}/include
        ${KERNEL_HEADERS_DIRECTORY}/arch/${PROCESSOR_ARCHITECTURE}/include
)

# Modify header files which aren't C++ conformant
function(_replace_in_file FILE_PATH REGEX REPLACEMENT)
    if (NOT EXISTS ${FILE_PATH})
        status(FATAL_ERROR "Attempted to replace data in ${FILE_PATH} which does not exist.")
    endif ()

    file(READ ${FILE_PATH} FILE_DATA)
    string(REGEX REPLACE "${REGEX}" "${REPLACEMENT}" FILE_DATA "${FILE_DATA}")
    file(WRITE ${FILE_PATH} "${FILE_DATA}")
endfunction()
