
cmake_minimum_required(VERSION 3.0)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/external)
project(lua)
include_directories("include")
file(GLOB srcs "include/*.h" "src/*.c")
add_library(lua ${srcs})

