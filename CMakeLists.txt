cmake_minimum_required(VERSION 3.10)
project(Dot)

# turn off compiler warnings
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
elseif(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++20")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++20")
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
    target_link_libraries(Dot glfw ${GLFW_LIBRARIES} Setupapi winmm)
    target_compile_definitions(Dot PRIVATE "-D__WINDOWS_MM__")
elseif(APPLE)
    #target_link_libraries(Dot ${GLFW_LIBRARIES} "-framework CoreAudio" "-framework CoreMIDI" "-framework CoreFoundation" "-framework IOKit")
    target_compile_definitions(Dot PRIVATE "-D__MACOSX_CORE__")
else()
    target_link_libraries(Dot glfw ${GLFW_LIBRARIES} rt pthread asound ${GTK3_LIBRARIES})
    target_compile_definitions(Dot PRIVATE "-D__LINUX_ALSA__")
endif()
