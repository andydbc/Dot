#include "dot.h"

#include "editor_view.h"

#include <iostream>
#include <map>

std::string window_title = "Dot";
int window_width = 1000;
int window_height = 600;
bool refresh = false;

void on_error(int action,const char* msg)
{
	std::cout << msg << std::endl;
}

void on_key(dot::window& w, int action, int mod, int key)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_ESCAPE)
			w.close();
		if (key == GLFW_KEY_F5)
			refresh = true;
		if (mod == GLFW_MOD_CONTROL && key == GLFW_KEY_S)
			w.get_view<editor_view>().get()->on_save();
	}
}

void update(dot::controller& c)
{
	c.execute(refresh);
	refresh = false;
}

auto parse_options(int argc, char* argv[])
{
	std::map<std::string, std::string> options;

	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "-s")
		{
			if (i + 1 < argc)
			{
				options[arg] = argv[i+1];
			}
		}
	}
	return options;
}

int main(int argc, char* argv[])
{
	auto devices = serial::list_ports();
	auto options = parse_options(argc, argv);

	dot::hardware hardware = { 14, 28, "" };
	dot::controller controller;
	
	if (!devices.empty())
		hardware.port = devices[0].port;

	controller.set_hardware(hardware);

	auto scriptIt = options.find("-s");
	if (scriptIt != options.end())
	{
		auto& scriptPath = scriptIt->second;
		controller.from_file(scriptPath);
	}

	dot::window window;
	
	window.initialize(window_width, window_height, window_title, on_key, on_error);
	window.set_view<editor_view>(&controller);
	
	float timeInterval = 1.0f / 60.0f;
	float lastTime = glfwGetTime();

	while (window.is_open())
	{
		update(controller);
		window.render();
		controller.send();
		while (glfwGetTime() < lastTime + timeInterval) { }
		lastTime += timeInterval;
	}

	return 0;
}