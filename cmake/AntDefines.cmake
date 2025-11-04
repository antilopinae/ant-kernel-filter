cmake_minimum_required(VERSION 3.26.0)

set(ANT_GLOBAL_DEFINITIONS
        ANT_PROGRAM_NAME="ant-kernel-filter"
        FEATURE_X_ENABLED
)

function(add_ant_global_definitions TARGET_NAME)
    target_compile_definitions(${TARGET_NAME}
            PRIVATE
            ${ANT_GLOBAL_DEFINITIONS}
    )
endfunction()