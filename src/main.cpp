#include "dot.h"

#include "editor_view.h"

#include <bitset>
#include <fstream>
#include <iostream>
#include <map>

std::string window_title = "Dot v" DOT_VERSION_STR;
int window_width = 1024;
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
			if (i + i < argc)
			{
				options[arg] = argv[i+1];
			}
		}
	}
	return options;
}

int main(int argc, char* argv[])
{
	auto options = parse_options(argc, argv);

	dot::controller controller(
		dot::hardware{ 14, 24, "/dev/ttyUSB0" }
	);

	auto scriptIt = options.find("-s");
	if (scriptIt != options.end())
	{
		auto& scriptPath = scriptIt->second;
		controller.from_file(scriptPath);
	}

	dot::window window;
	
	window.initialize(window_width, window_height, window_title, on_key, on_error);
	window.set_view<editor_view>(&controller);
	
	float timeInterval = 1.0f / 30.0f;
	float time = 0.0f;

	while (window.is_open())
	{
		time += glfwGetTime();

		if (time > timeInterval)
		{
			update(controller);
			time = 0.0f;
		}

		window.render();
	}

	return 0;
}

/*#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#if defined(DOT_WINDOWS)
#include <windows.h>
#include "nfd.h"
#elif defined(DOT_UNIX)
#include <unistd.h>
#endif

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "serial/serial.h"

#include "display.h"
#include "perlin.h"

const uint32_t width = 1024;
const uint32_t height = 600;
const uint32_t pixel_rows = 14;
const uint32_t pixel_columns = 28;
const ImU32 black = ImColor(ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
const ImU32 white = ImColor(ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
const ImU32 red = ImColor(ImVec4(1.0f, 0.0f, 0.1f, 1.0f));

int frame = 0;
float period = 0.0f;
float prev_time = 0.0f;
char editBuffer[1024 * 16] = "\0";
char execBuffer[1024 * 16] = "\0";
bool hasErrors = false;
bool unsavedModifications = false;
std::string filepath;

display _display(pixel_rows, pixel_columns);

static int lua_print(lua_State* L) 
{
	int nargs = lua_gettop(L);
	for (int i = 1; i <= nargs; ++i) {
		std::cout << lua_tostring(L, i);
	}
	std::cout << std::endl;

	return 0;
}

static const struct luaL_Reg printlib[] = {
  {"print", lua_print},
  {NULL, NULL}
};

std::string get_current_folder()
{
#ifdef _WIN32
#include <windows.h>
	char result[MAX_PATH];
	GetModuleFileName(NULL, result, MAX_PATH);
	std::string::size_type pos = std::string(result).find_last_of("\\/");
	return std::string(result).substr(0, pos);
#elif __unix__
#include <limits.h>
#include <unistd.h>
	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	return std::string(result, (count > 0) ? count : 0);
#endif
}

bool compile(lua_State* lua, std::string& ret_error)
{
	memcpy(execBuffer, editBuffer, sizeof(char) * 1024 * 16);
	int err = luaL_dostring(lua, execBuffer);
	if (err != 0)
		ret_error = lua_tostring(lua, -1);
	return err ==0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, true);
	else if (action == GLFW_PRESS && key == GLFW_KEY_F5)
	{
		lua_State* lua = (lua_State*)glfwGetWindowUserPointer(window);
		std::string error;
		if (compile(lua, error))
		{
			hasErrors = false;
			std::cout << "Compilation Successful\n";
			frame = 0;
		}
		else
		{
			hasErrors = true;
			std::cout << "Compilation Failed ";
			std::cout << "With Errors: " + error + "\n";
		}
	}
	else if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL && key == GLFW_KEY_N)
	{
		memset(editBuffer, 0, sizeof(char) * 1024 * 16);
		memset(execBuffer, 0, sizeof(char) * 1024 * 16);
	}
	else if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL && key == GLFW_KEY_O)
	{
#ifdef _WIN32
		nfdchar_t* outPath = NULL;
		nfdresult_t result = NFD_OpenDialog("lua", get_current_folder().c_str(), &outPath);
#endif
	}
	else if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL && key == GLFW_KEY_S)
	{
#ifdef _WIN32
		if (filepath.empty())
		{
			nfdchar_t *outPath = NULL;
			nfdresult_t result = NFD_SaveDialog("lua", get_current_folder().c_str(), &outPath);
			if (result == NFD_OKAY)
			{
				filepath = outPath;
				if (filepath.substr(filepath.find_last_of(".") + 1) != "lua")
					filepath += ".lua";
			}
		}
		
		if (!filepath.empty())
		{
			std::ofstream script;
			script.open(filepath);
			script << editBuffer;
			script.close();
			unsavedModifications = false;
		}
#endif
	}
}


#include <chrono>
#include <thread>

#ifdef _WIN32
void send_msg()
#else
void send_msg(serial::Serial& serial)
#endif
{
	int panel_width = 7;
	int num_panels = pixel_rows / panel_width;

	for (int p = 0; p < num_panels; ++p)
	{
		int panel = p;
			
		std::bitset<8> panelmask;
		panelmask[(num_panels-1)-panel] = 1;
			
		std::vector<unsigned char> msg;
		msg.push_back(0x80);
		msg.push_back(0x83);
		msg.push_back((unsigned char)panelmask.to_ulong());

		for (uint32_t y = 0; y < pixel_columns; ++y)
		{
			std::bitset<8> bitmask;
				
			for (uint32_t x = 0; x < pixel_rows / 2; ++x)
			{
				int xx = x + (panel_width * panel);
				bitmask[x] = _display.get_pixel(xx, y);
			}

			unsigned long i = bitmask.to_ulong();
			msg.push_back((unsigned char)i);
				
		}
	
		msg.push_back(0x8F);
#ifndef _WIN32
		serial.write(&msg[0], msg.size());
#endif
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

int main(int argc, char* argv[])
{

#ifndef _WIN32
	serial::Serial serial("/dev/ttyUSB0", 9600, serial::Timeout::simpleTimeout(1000));
	if (serial.isOpen())
	{
		std::cout << "Serial Connected" << std::endl;
	}
#endif

	memcpy(editBuffer, execBuffer, sizeof(char) * 1024 * 16);
	luaL_dostring(lua, execBuffer);

#ifdef _WIN32
	while (!glfwWindowShouldClose(window))
#else
	while(1)
#endif
	{
		float time = glfwGetTime();
		float dt = time - prev_time;

		glfwPollEvents();

		period += dt;
		update_pixels(lua);

#ifdef _WIN32
		send_msg();
#else
		send_msg(serial);
#endif

#ifdef _WIN32
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		{
			ImGui::SetNextWindowPos(ImVec2(35, 20), ImGuiCond_Always, ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(500, 465), ImGuiCond_Always);
			ImGui::Begin("TextEdit", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
			{
				if (ImGui::InputTextMultiline("", editBuffer, IM_ARRAYSIZE(editBuffer), ImVec2(-FLT_MIN, 500), ImGuiInputTextFlags_AllowTabInput))
				{
					unsavedModifications = true;
				}
				ImGui::End();
			}

			ImGui::SetNextWindowPos(ImVec2(35, 500), ImGuiCond_Always, ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(400, 100), ImGuiCond_Always);
			ImGui::Begin("Compile", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
			{
				std::string filename = "Untitled";
				ImVec4 col = ImVec4(0.26f, 0.26f, 0.26f, 1.0f);
				if (!filepath.empty())
				{
					filename = filepath.substr(filepath.find_last_of("\\")+1);
					if (unsavedModifications)
						filename += '*';
					col = ImVec4(0.65f, 0.65f, 0.65f, 1.0f);
				}

				ImGui::PushStyleColor(ImGuiCol_Text, col);
				ImGui::LabelText("", filename.c_str());
				ImGui::PopStyleColor();

				ImDrawList* draw_list = ImGui::GetWindowDrawList();
				ImVec2 wp = ImGui::GetWindowPos();
				draw_list->AddCircleFilled(ImVec2(wp.x + 20, wp.y + 55), 12.0f, hasErrors ? red : white, 64);
				ImGui::End();
			}
			
			ImGui::SetNextWindowPos(ImVec2(424, 0), ImGuiCond_Always, ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(600, 600), ImGuiCond_Always);

			ImGui::Begin("Preview", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);
			{
				float radius = 8.0f;
				float dradius = radius * 2.0f;
				float padding = 2.0f;
				int width = pixel_rows * (dradius + padding);
				int height = pixel_columns * (dradius + padding);

				ImDrawList* draw_list = ImGui::GetWindowDrawList();
				ImVec2 wp = ImGui::GetWindowPos();
				ImVec2 ws = ImGui::GetWindowSize();
				
				for (uint32_t j = 0; j < pixel_columns; ++j)
				{
					for (uint32_t i = 0; i < pixel_rows; ++i)
					{
						float x = (wp.x + dradius) + i * (dradius + padding) + ws.x * 0.5f - width * 0.5f;
						float y = (wp.y + dradius) + j * (dradius + padding) + ws.y * 0.5f - height * 0.5f;
						int pixel = _display.get_pixel(i, j);
						int pixelColor = pixel ? white : black;
						draw_list->AddCircleFilled(ImVec2(x, y-20), radius, pixelColor, 64);
					}
				}

				ImGui::End();
			}
		}

		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
#endif
		period += dt;
		prev_time = time;
	}


	return 0;
}*/