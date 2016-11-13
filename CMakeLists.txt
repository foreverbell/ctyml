cmake_minimum_required (VERSION 3.0)

project (ctyml CXX)

# Feed `compile_commands.json` to ycm.
set (CMAKE_EXPORT_COMPILE_COMMANDS "ON")

add_subdirectory (deps/gtest)
add_definitions (-Wall -std=c++11)

file (GLOB SOURCE_FILES src/*.cc)
add_executable (ctyml ${SOURCE_FILES})

# Tests
enable_testing ()

file (GLOB TEST_FILES tests/*.cc)
add_executable (ctyml_test ${TEST_FILES})
add_dependencies (ctyml_test gtest)
target_link_libraries (ctyml_test gtest_main)
add_test (ctyml_test ctyml_test)
