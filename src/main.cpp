#include <bitset>
#include <iostream>
#include <fstream>
#include <vector>

#include "cxxopts.hpp"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#ifdef _WIN32
#include "nfd.h"
#endif

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "serial/serial.h"

#include "display.h"
#include "perlin.h"

const uint32_t width = 1024;
const uint32_t height = 600;
const uint32_t pixel_rows = 14;
const uint32_t pixel_columns = 28;
const ImU32 black = ImColor(ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
const ImU32 white = ImColor(ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
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

void apply_custom_style()
{
#ifdef _WIN32
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive]	= ImVec4(0.45f, 0.45f, 0.45f, 1.0f);
	style.Colors[ImGuiCol_WindowBg]	= ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.65f, 0.65f, 0.65f, 1.0f);
	style.ScrollbarRounding	= 0;
	style.WindowBorderSize = 0;
	style.WindowRounding = 0;
#endif
}

void update_pixels(lua_State* lua)
{
	if (period >= 0.25f && !hasErrors)
	{
		for (uint32_t j = 0; j < pixel_columns; ++j)
		{
			for (uint32_t i = 0; i < pixel_rows; ++i)
			{
				lua_getglobal(lua, "main");
				lua_pushinteger(lua, i);
				lua_pushinteger(lua, j);
				lua_pushinteger(lua, frame);

				if (lua_pcall(lua, 3, 1, 0) != 0)
				{
					std::cout << "Execution error" << std::endl;
					hasErrors = true;
				}

				_display.set_pixel(i, j, 0);

				if (lua_isinteger(lua, -1))
				{
					int pixel = lua_tointeger(lua, -1);
					lua_pop(lua, 1);

					_display.set_pixel(i, j, pixel);
				}
			}
		}

		frame++;
		period = 0;
	}
}

void send_msg(serial::Serial& serial)
{
#ifndef _WIN32
	if (serial.isOpen())
	{
		int panel_width = 7;
		int num_panels = pixel_rows / panel_width;

		std::vector<unsigned char> msg;
		msg.push_back(0x80);
		msg.push_back(0x83);
		msg.push_back(0xFF);

		for (int p = 0; p < num_panels; ++p)
		{
			for (uint32_t y = 0; y < pixel_columns; ++y)
			{
				std::bitset<7> bitmask;
				for (uint32_t x = 0; x < panel_width; ++x)
				{
					bitmask[x] = _display.get_pixel(x, y);
				}

				unsigned long i = bitmask.to_ulong();
				msg.push_back((unsigned char)i);
			}
		}

		msg.push_back(0x8F);
		serial.write(&msg[0], msg.size());
	}
#endif // !_WIN32
}

int main(int argc, char* argv[])
{

	glfwInit();
	
#ifdef _WIN32
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
#else
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#endif
	
	glfwWindowHint(GLFW_RESIZABLE, false);

	GLFWwindow* window = glfwCreateWindow(width, height, "Dot", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
#ifdef _WIN32
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigInputTextCursorBlink = true;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 
	io.Fonts->AddFontFromFileTTF("resources/input_mono_regular.ttf", 20.0f);

	const char* glsl_version = "#version 460";
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	apply_custom_style();
	ImVec4 clear_color = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
#endif

	lua_State* lua = luaL_newstate();
	luaL_openlibs(lua);
	lua_getglobal(lua, "_G");
	luaL_setfuncs(lua, printlib, 0);
	lua_pop(lua, 1);

	register_perlin(lua);

	glfwSetWindowUserPointer(window, lua);

	static char execBuffer[1024 * 16] = "-- welcome to Dot - v0.1 \n\n"
		"function all_whites()\n"
		"	return 1 \n"
		"end\n\n"
		"function all_blacks()\n"
		"	return 0\n"
		"end\n\n"
		"function main(x, y, frame)\n"
		"	return all_whites()\n"
		"end";


	// Options parsing
	cxxopts::Options options(argv[0], " - Command line options");
	options.add_options()
		("s,script", "Script", cxxopts::value<std::string>())
		;

	auto result = options.parse(argc, argv);

	if (result.count("script"))
	{
		auto& script = result["s"].as<std::string>();
		std::ifstream file(script);
		if (file)
		{
			memset(&execBuffer[0], 0, sizeof(char) * 1024 * 16);
			file.seekg(0, std::ios::end);
			std::streampos length = file.tellg();
			file.seekg(0, std::ios::beg);
			file.read(&execBuffer[0], length);
			filepath = script;
		}
	}

#ifndef _WIN32
	serial::Serial serial("/dev/ttyUSB0", 9600, serial::Timeout::simpleTimeout(1000));
#endif

	memcpy(editBuffer, execBuffer, sizeof(char) * 1024 * 16);
	luaL_dostring(lua, execBuffer);

	while (!glfwWindowShouldClose(window))
	{
		float time = glfwGetTime();
		float dt = time - prev_time;

		glfwPollEvents();

		period += dt;
		update_pixels(lua);

#ifndef _WIN32
		send_msg(serial);
#endif

#ifdef _WIN32
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		{
			ImGui::SetNextWindowPos(ImVec2(35, 20), ImGuiCond_Always, ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(400, 465), ImGuiCond_Always);
			ImGui::Begin("TextEdit", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
			{
				if (ImGui::InputTextMultiline("", editBuffer, IM_ARRAYSIZE(editBuffer), ImVec2(-FLT_MIN, 420), ImGuiInputTextFlags_AllowTabInput))
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

			ImGui::Begin("Preview", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
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

#endif
		glfwSwapBuffers(window);
		period += dt;
		prev_time = time;
	}

	// Clean-up lua
	lua_close(lua);

#ifdef _WIN32
	// Clean-up imgui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
#endif

	// Clean-up GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}