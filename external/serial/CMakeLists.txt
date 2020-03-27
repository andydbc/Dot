
cmake_minimum_required(VERSION 3.0)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/external)
project(serial)
include_directories("include")
file(GLOB srcs "include/serial/*.h" "src/*.cc")

if(UNIX)
    list(APPEND srcs src/impl/unix.cc)
    list(APPEND srcs src/impl/list_ports/list_ports_linux.cc)
else()
    list(APPEND srcs src/impl/win.cc)
    list(APPEND srcs src/impl/list_ports/list_ports_win.cc)
endif()

add_library(serial ${srcs})
