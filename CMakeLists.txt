cmake_minimum_required (VERSION 3.0)

project (ctyml CXX)

# Feed `compile_commands.json` to ycm.
set (CMAKE_EXPORT_COMPILE_COMMANDS "ON")

add_subdirectory (deps/gtest)
add_definitions (-Wall -std=c++1z)

if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
  set (CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)
  set (CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
  add_custom_target (ctyml_coverage ./test-coverage.sh)
else ()
  add_definitions (-O2)
endif ()

file (GLOB SOURCE_FILES src/*.cc)
list (REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc)
add_library (ctyml_lib ${SOURCE_FILES})

add_executable (ctyml ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc)
add_dependencies (ctyml ctyml_lib)
target_link_libraries (ctyml ctyml_lib)

# Tests.
enable_testing ()
include_directories (${GTEST_INCLUDE_DIRS} src/)
file (GLOB TEST_FILES tests/*.cc)
add_executable (ctyml_test ${TEST_FILES})
add_dependencies (ctyml_test ctyml_lib gtest)
target_link_libraries (ctyml_test ctyml_lib gtest_main)
add_test (ctyml_test ctyml_test)
