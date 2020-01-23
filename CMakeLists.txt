cmake_minimum_required(VERSION 3.0)
project(Dot)

# based largely on: 
# https://github.com/Polytonic/Glitter/blob/master/CMakeLists.txt

# turn off compiler warnings
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++11")
endif()

# setup GLFW CMake project
add_subdirectory("${PROJECT_SOURCE_DIR}/external/glfw")

# include headers
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/external/glfw/include/GLFW")
include_directories("${PROJECT_SOURCE_DIR}/external/imgui/include")
include_directories("${PROJECT_SOURCE_DIR}/external/glad/include")
include_directories("${PROJECT_SOURCE_DIR}/external/lua/include")
include_directories("${PROJECT_SOURCE_DIR}/external/serial/include")
include_directories("${PROJECT_SOURCE_DIR}/external/sfd/include")
include_directories("${PROJECT_SOURCE_DIR}/external/glm/glm")
file(GLOB PROJECT_HEADERS "include/*.h")

# include source files
file(GLOB PROJECT_SOURCES "src/*.cpp")
file(GLOB IMGUI_SOURCES "external/imgui/src/*.cpp")
file(GLOB GLAD_SOURCES "external/glad/src/*.c")
file(GLOB LUA_SOURCES "external/lua/src/*.c")
file(GLOB SERIAL_SOURCES "external/serial/src/*.cc")
file(GLOB SFD_SOURCES "external/sfd/src/*.c")

if(UNIX)
    # If unix
    list(APPEND SERIAL_SOURCES external/serial/src/impl/unix.cc)
    list(APPEND SERIAL_SOURCES external/serial/src/impl/list_ports/list_ports_linux.cc)
else()
    # If windows
    list(APPEND SERIAL_SOURCES external/serial/src/impl/win.cc)
    list(APPEND SERIAL_SOURCES external/serial/src/impl/list_ports/list_ports_win.cc)
endif()

# group files in IDE
source_group("include" FILES ${PROJECT_HEADERS})
source_group("src" FILES ${PROJECT_SOURCES})
source_group("external" FILES ${IMGUI_SOURCES} ${GLAD_SOURCES} ${LUA_SOURCES} ${SERIAL_SOURCES} ${SFD_SOURCES})

# create the executable
add_executable(Dot ${PROJECT_SOURCES}
					${PROJECT_HEADERS} # if not included here, it won't show up in the IDE?
					${IMGUI_SOURCES}
					${GLAD_SOURCES}
					${LUA_SOURCES}
					${SERIAL_SOURCES}
					${SFD_SOURCES})

# add libraries

if(UNIX)
    target_link_libraries(Dot glfw ${GLFW_LIBRARIES} rt pthread)
else()
    target_link_libraries(Dot glfw ${GLFW_LIBRARIES} setupapi)
endif()