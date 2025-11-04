cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

find_package(GTest CONFIG REQUIRED)
enable_testing()

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