cmake_minimum_required (VERSION 3.0)

project (ctyml CXX)

add_subdirectory (deps/gtest)

file(GLOB SOURCE_FILES src/*.cc)
add_executable(ctyml ${SOURCE_FILES})
