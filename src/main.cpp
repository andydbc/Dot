#include <iostream>
#include <fstream>
#include <vector>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "nfd.h"

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

const uint32_t width = 1024;
const uint32_t height = 600;
const uint32_t pixel_rows = 14;
const uint32_t pixel_columns = 28;
const ImU32 black = ImColor(ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
const ImU32 white = ImColor(ImVec4(0.8f, 0.8f, 0.8f, 1.0f));

int frame = 0;
float period = 0.0f;
float prev_time = 0.0f;
char editBuffer[1024 * 16] = "\0";
char execBuffer[1024 * 16] = "\0";
bool error = false;
bool play = false;
bool unsavedModifications = false;
std::string filepath;

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

void compile(lua_State* lua)
{
	memcpy(execBuffer, editBuffer, sizeof(char) * 1024 * 16);
	int err = luaL_dostring(lua, execBuffer);

	error = false;
	if (err != 0)
	{
		std::cout << lua_tostring(lua, -1) << std::endl;
		play = false;
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, true);
	else if (action == GLFW_PRESS && key == GLFW_KEY_P && mods == GLFW_MOD_CONTROL)
	{
		play = !play;
		if (play)
		{
			lua_State* lua = (lua_State*)glfwGetWindowUserPointer(window);
			compile(lua);
		}
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_S && mods == GLFW_MOD_CONTROL)
	{
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
	}
}

void apply_custom_style()
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg]			= ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
	style.Colors[ImGuiCol_ChildBg]			= ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
	style.Colors[ImGuiCol_FrameBg]			= ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
	style.Colors[ImGuiCol_TextSelectedBg]	= ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
	style.Colors[ImGuiCol_Button]			= ImVec4(0.65f, 0.65f, 0.65f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered]	= ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive]		= ImVec4(0.45f, 0.45f, 0.45f, 1.0f);
	style.WindowRounding					= 0;
	style.WindowBorderSize					= 0;
	style.ScrollbarRounding					= 0;
}

void update_pixels(lua_State* lua, std::vector<int>& pixels)
{
	if (!play) return;

	if (period >= 0.1f)
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
					std::cout << "Error" << std::endl;
					error = true;
				}

				int index = i + pixel_rows * j;
				pixels[index] = 0;

				if (lua_isinteger(lua, -1))
				{
					int pixel = lua_tointeger(lua, -1);
					lua_pop(lua, 1);
					pixels[index] = pixel;
				}
			}
		}

		frame++;
		period = 0;
	}
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, false);
	glfwWindowHint(GLFW_SAMPLES, 4);

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

	lua_State* lua = luaL_newstate();
	luaL_openlibs(lua);
	lua_getglobal(lua, "_G");
	luaL_setfuncs(lua, printlib, 0);
	lua_pop(lua, 1);

	glfwSetWindowUserPointer(window, lua);

	std::vector<int> pixels(pixel_rows * pixel_columns);

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
	
	memcpy(editBuffer, execBuffer, sizeof(char)*1024*16);
	luaL_dostring(lua, execBuffer);

	while (!glfwWindowShouldClose(window))
	{
		float time = glfwGetTime();
		float dt = time - prev_time;

		glfwPollEvents();

		period += dt;
		update_pixels(lua, pixels);

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
				draw_list->AddCircleFilled(ImVec2(wp.x + 20, wp.y + 55), 12.0f, white, 64);
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
						int pixelColor = pixels[i + pixel_rows*j] ? white : black;
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

		period += dt;
		prev_time = time;
	}

	// Clean-up lua
	lua_close(lua);

	// Clean-up imgui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	   
	// Clean-up GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}