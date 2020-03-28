#pragma once

#include "common.h"

#include <memory>
#include <string>

DOT_NS_BEGIN

class view;

struct color
{
	float r, g, b, a;

	static color black;
	static color white;
};

class window
{
public:

	typedef void(onKey)(window&, int, int, int);
	typedef void(onError)(int, const char*);

	void initialize(int w, int h, const std::string& title, onKey& on_key, onError& on_error);
	void close();
	
	void render();

	bool is_open();
	void events();

	template<class R, typename ...Ts>
	std::shared_ptr<R> set_view(Ts... args) 
	{
		auto v = std::make_shared<R>(args...);
		
		_view = v;
		_view->initialize(*this);

		return v;
	}

	void set_clear_color(const color& c) {
		_clear_color = c;
	}

	bool key_pressed(int code) {
		return glfwGetKey(_window.get(), code) == GLFW_PRESS;
	}

private:
	
	onKey* _on_key;

private:
	static void _key_cb(WindowPtr window, int key, int scancode, int action, int mods);

	std::shared_ptr<Window> _window;
	std::shared_ptr<view> _view;
	color _clear_color;
};

DOT_NS_END