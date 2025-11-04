cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

include(Testing REQUIRED)

link_libraries(util cunit pthread)
add_compile_definitions($<$<CONFIG:Debug>:DEBUG_BUILD>)
add_compile_options(-Wall -Wextra -Wpedantic)

#target_link_libraries(main PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

#add_test(AllTestsInMain main)

#add_executable(${PROJECT_NAME}-tests
#)
#
#target_include_directories(${PROJECT_NAME}-tests PUBLIC
#        ${CMAKE_CURRENT_SOURCE_DIR}
#)
#
#target_link_libraries(${PROJECT_NAME}-tests PUBLIC
#        GTest::gtest
#        GTest::gtest_main
#)
#
#add_test(NAME RunAllTests COMMAND ${PROJECT_NAME}-tests)