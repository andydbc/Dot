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

	serial::Serial serial;

	if(!devices.empty())
	{
		std::cout << "Found serial devices:" << std::endl;
		for (const auto& device : devices)
		{
			std::cout << " - " << device.port << " : " << device.description << std::endl;
		}

		std::cout << "Using port: " << devices[0].port << std::endl;
		serial.setPort(devices[0].port);
	}
	else
	{
		std::cout << "No serial devices found." << std::endl;
	}


	dot::controller controller(14, 28);

	auto scriptIt = options.find("-s");
	if (scriptIt != options.end())
	{
		std::cout << "Loading script: " << scriptIt->second << std::endl;
		auto& scriptPath = scriptIt->second;
		controller.from_file(scriptPath);
	}

	dot::window window;
	
	window.initialize(window_width, window_height, window_title, on_key, on_error);
	window.set_view<editor_view>(&controller);
	
	float timeInterval = 1.0f / 30.0f;
	float lastTime = glfwGetTime();

	while (window.is_open())
	{
		update(controller);
		
		window.render();
		if(serial.isOpen())
			controller.send(serial);

		while (glfwGetTime() < lastTime + timeInterval) { }
		lastTime += timeInterval;
	}

	return 0;
}