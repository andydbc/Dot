cmake_minimum_required(VERSION 3.0)
project(Dot)

# turn off compiler warnings
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++14")
endif()

add_subdirectory("${PROJECT_SOURCE_DIR}/external/glfw")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/glad")
add_subdirectory("${PROJECT_SOURCE_DIR}/external/lua")

include_directories("${PROJECT_SOURCE_DIR}/external/glad/include")
include_directories("${PROJECT_SOURCE_DIR}/external/lua/include")
include_directories("${PROJECT_SOURCE_DIR}/external/imgui/include")

file(GLOB PROJECT_FILES "src/*.h" "src/*.cpp")

# include headers
#include_directories("${PROJECT_SOURCE_DIR}/external/imgui/include")
#include_directories("${PROJECT_SOURCE_DIR}/external/lua/include")
#include_directories("${PROJECT_SOURCE_DIR}/external/serial/include")

file(GLOB IMGUI_SOURCES "external/imgui/src/*.cpp")
#file(GLOB LUA_SOURCES "external/lua/src/*.c")
#file(GLOB SERIAL_SOURCES "external/serial/src/*.cc")

#include_directories("${PROJECT_SOURCE_DIR}/external/nfd/include")
#include_directories("${PROJECT_SOURCE_DIR}/external/nfd/src")
#file(GLOB NFD_SOURCES "external/nfd/src/nfd_common.c")
#if(UNIX)
#    list(APPEND SERIAL_SOURCES external/serial/src/impl/unix.cc)
#    list(APPEND SERIAL_SOURCES external/serial/src/impl/list_ports/list_ports_linux.cc)
#else()
#    list(APPEND NFD_SOURCES "external/nfd/src/nfd_win.cpp")
#    list(APPEND SERIAL_SOURCES external/serial/src/impl/win.cc)
#    list(APPEND SERIAL_SOURCES external/serial/src/impl/list_ports/list_ports_win.cc)
#endif()

# create the executable
source_group("external" FILES ${IMGUI_SOURCES})
add_executable(Dot ${PROJECT_FILES} ${IMGUI_SOURCES})

target_link_libraries(Dot glfw ${GLFW_LIBRARIES} glad lua)

# add libraries

#if(UNIX)
#    target_link_libraries(Dot glfw ${GLFW_LIBRARIES} rt pthread ${GTK3_LIBRARIES})
#else()
#    target_link_libraries(Dot glfw ${GLFW_LIBRARIES} setupapi)
#endif()