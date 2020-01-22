#include <iostream>
#include <vector>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
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

int frame = 0;
float period = 0.0f;
float prev_time = 0.0f;
char buffer[1024 * 16] = "\0";
bool error = false;

void update_pixels(lua_State* lua, std::vector<int>& pixels)
{
	if (period >= 0.1f && !error)
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
					std::string message = std::string("error running function 'f': %s", lua_tostring(lua, -1));
					std::cout << message << std::endl;
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
	serial::Serial usbSerial("/dev/ttyUSB0", 9600, serial::Timeout::simpleTimeout(1000));
	if (usbSerial.isOpen())
	{
		std::cout << "Serial Connected" << std::endl;
		unsigned char all_white[32] =
		{
			0x80,
			0x83,
			0x01,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
			0x8F
		};

		usbSerial.write(&all_white[0], 32);
	}

	/*glfwInit();
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
	io.Fonts->AddFontFromFileTTF("resources/input_mono_medium.ttf", 20.0f);

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

	std::vector<int> pixels(pixel_rows * pixel_columns);

	static char text[1024 * 16] = "-- welcome to Dot - v0.1 \n\n"
		"function all_whites()\n"
		"	return 1 \n"
		"end\n\n"
		"function all_blacks()\n"
		"	return 0\n"
		"end\n\n"
		"function main(x, y, frame)\n"
		"	return all_whites()\n"
		"end";
	
	memcpy(buffer, text, sizeof(char)*1024*16);
	luaL_dostring(lua, buffer);

	while (!glfwWindowShouldClose(window))
	{
		float time = glfwGetTime();
		float dt = time - prev_time;

		glfwPollEvents();

		period += dt;
		update_pixels(lua, pixels);

		//////
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		{
			ImGui::SetNextWindowPos(ImVec2(35, 35), ImGuiCond_Always, ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(400, 465), ImGuiCond_Always);
			ImGui::Begin("TextEdit", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
			{
				ImGui::InputTextMultiline("", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, 420), ImGuiInputTextFlags_AllowTabInput);
				ImGui::End();
			}

			ImGui::SetNextWindowPos(ImVec2(35, 500), ImGuiCond_Always, ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(400, 100), ImGuiCond_Always);
			ImGui::Begin("Compile", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
			{
				if (ImGui::Button("Execute"))
				{
					memcpy(buffer, text, sizeof(char) * 1024 * 16);
					int err = luaL_dostring(lua, buffer);
					if (err != 0)
					{
						std::cout << lua_tostring(lua, -1) << std::endl;
						error = true;
					}
					else
					{
						error = false;
					}
				}

				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
				ImGui::LabelText("", "Untitled");
				ImGui::PopStyleColor();
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
				ImVec2 wc = ImVec2(wp.x + ws.x * 0.5f, wp.y + ws.y * 0.5f);

				for (uint32_t j = 0; j < pixel_columns; ++j)
				{
					for (uint32_t i = 0; i < pixel_rows; ++i)
					{
						float x = (wp.x + dradius) + i * (dradius + padding) + ws.x * 0.5f - width * 0.5f;
						float y = (wp.y + dradius) + j * (dradius + padding) + ws.y * 0.5f - height * 0.5f;
						int pixelColor = pixels[i + pixel_rows*j] ? white : black;
						draw_list->AddCircleFilled(ImVec2(x, y), radius, pixelColor, 64);
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
	glfwTerminate();*/

	return 0;
}