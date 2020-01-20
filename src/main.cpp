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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

const uint32_t width = 1024;
const uint32_t height = 600;

static int lua_print(lua_State* L) {
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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load function pointers from GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    const char* glsl_version = "#version 460";
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImFont* pFont = io.Fonts->AddFontFromFileTTF("resources/input_mono_medium.ttf", 20.0f);

	io.ConfigInputTextCursorBlink = true;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.65f, 0.65f, 0.65f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.45f, 0.45f, 0.45f, 1.0f);
	style.WindowRounding = 0;
	style.WindowBorderSize = 0;
	style.ScrollbarRounding = 0;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);

	ImGuiTextBuffer log;
	glfwSetWindowUserPointer(window, &log);

	lua_State* lua = luaL_newstate();
	luaL_openlibs(lua);
	lua_getglobal(lua, "_G");
	luaL_setfuncs(lua, printlib, 0);
	lua_pop(lua, 1);

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
	
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
			ImGui::SetNextWindowPos(ImVec2(35, 35), ImGuiCond_Always, ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(400, 465), ImGuiCond_Always);
            ImGui::Begin("TextEdit", NULL, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse);
			
			ImGui::InputTextMultiline("", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, 420), ImGuiInputTextFlags_AllowTabInput);
			ImGui::End();

			ImGui::SetNextWindowPos(ImVec2(35, 500), ImGuiCond_Always, ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(400, 100), ImGuiCond_Always);
			ImGui::Begin("Compile", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
			if (ImGui::Button("Execute"))
			{
				luaL_dostring(lua, text);
			}
			ImGui::End();
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
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

