#pragma once

// Platform
#ifdef _MSC_VER
	#define DOT_WINDOWS
#else
	#define DOT_UNIX
#endif

// Namespaces
#define DOT_NS_BEGIN namespace dot {
#define DOT_NS_END }

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Version MAJOR.MINOR.PATCH
#define DOT_VERSION_MAJOR 0
#define DOT_VERSION_MINOR 2
#define DOT_VERSION_PATCH 3

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define DOT_VERSION_STR STR(DOT_VERSION_MAJOR.DOT_VERSION_MINOR.DOT_VERSION_PATCH)

using Window = GLFWwindow;
typedef Window* WindowPtr;