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
add_subdirectory("${PROJECT_SOURCE_DIR}/external/serial")

include_directories("${PROJECT_SOURCE_DIR}/external/glad/include")
include_directories("${PROJECT_SOURCE_DIR}/external/lua/include")
include_directories("${PROJECT_SOURCE_DIR}/external/imgui/include")
include_directories("${PROJECT_SOURCE_DIR}/external/serial/include")

file(GLOB PROJECT_FILES "src/*.h" "src/*.cpp")
file(GLOB IMGUI_SOURCES "external/imgui/src/*.cpp")

# create the executable
source_group("external" FILES ${IMGUI_SOURCES})
add_executable(Dot ${PROJECT_FILES} ${IMGUI_SOURCES})

# add libraries
target_link_libraries(Dot glfw ${GLFW_LIBRARIES} glad lua serial)

if(MSVC)
    target_link_libraries(Dot glfw ${GLFW_LIBRARIES} Setupapi)
else()
    target_link_libraries(Dot glfw ${GLFW_LIBRARIES} rt pthread ${GTK3_LIBRARIES})
endif()