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

using Window = GLFWwindow;
typedef Window* WindowPtr;