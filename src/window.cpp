#include "window.h"
#include "view.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>

DOT_NS_BEGIN

color color::black = { 0.0,0.0,0.0,1.0 };
color color::white = { 1.0,1.0,1.0,1.0 };

void window::initialize(int w, int h, const std::string& title, onKey& on_key, onError& on_error)
{
	glfwSetErrorCallback(on_error);
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE, false);

	_window = std::shared_ptr<Window>(
		glfwCreateWindow(w, h, title.c_str(), NULL, NULL),
		glfwDestroyWindow
		);

	if (_window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(_window.get());
	glfwSetKeyCallback(_window.get(), window::_key_cb);
	glfwSetWindowUserPointer(_window.get(), this);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	const char* glsl_version = "#version 460";
	ImGui_ImplGlfw_InitForOpenGL(_window.get(), true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigInputTextCursorBlink = true;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	_on_key = *on_key;
}

void window::close()
{
	glfwSetWindowShouldClose(_window.get(), true);
}

int window::get_width()
{
	int w, h;
	glfwGetWindowSize(_window.get(), &w, &h);
	return w;
}

int window::get_height()
{
	int w, h;
	glfwGetWindowSize(_window.get(), &w, &h);
	return h;
}

void window::render()
{
	glfwPollEvents();

	if (_view != nullptr)
	{
		_view->on_render(*this);
	}

	ImGui::Render();

	int w, h;
	glfwGetFramebufferSize(_window.get(), &w, &h);
	glViewport(0, 0, w, h);
	glClearColor(_clear_color.r, _clear_color.g, _clear_color.b, _clear_color.a);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(_window.get());
}

bool window::is_open()
{
	return !glfwWindowShouldClose(_window.get());
}

void window::events()
{
	glfwPollEvents();
}

void window::_key_cb(WindowPtr w, int key, int scancode, int action, int mods)
{
	auto owner = (window*)glfwGetWindowUserPointer(w);
	owner->_on_key((*owner), action, mods, key);
}

DOT_NS_END