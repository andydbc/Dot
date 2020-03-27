#include "dot.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class main_view : public dot::view
{
public:
	main_view(dot::controller* c) : _controller(c) { }

	void initialize() override
	{
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->AddFontFromFileTTF("resources/input_mono_regular.ttf", 20.0f);

		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.45f, 0.45f, 0.45f, 1.0f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.65f, 0.65f, 0.65f, 1.0f);
		style.ScrollbarRounding = 0;
		style.WindowBorderSize = 0;
		style.WindowRounding = 0;
	}

	void on_render(dot::window& w) override
	{
 		dot::color clear_color = { 0.06, 0.06, 0.06, 1.0 };
		w.set_clear_color(clear_color);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		{
			ImGui::SetNextWindowPos(ImVec2(35, 20), ImGuiCond_Always, ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(500, 465), ImGuiCond_Always);
			ImGui::Begin("TextEdit", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
			{
				_controller->get_script(_buffer);
				ImGui::InputTextMultiline("", &_buffer[0], _buffer.size(), ImVec2(500, 500), ImGuiInputTextFlags_AllowTabInput);
				ImGui::End();
				_controller->set_script(_buffer);
			}
		}

		ImGui::SetNextWindowPos(ImVec2(35, 500), ImGuiCond_Always, ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(400, 100), ImGuiCond_Always);
		ImGui::Begin("Info", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		{
			std::string filename = "Untitled";
			ImVec4 col = ImVec4(0.26f, 0.26f, 0.26f, 1.0f);

			ImGui::PushStyleColor(ImGuiCol_Text, col);
			ImGui::LabelText("", filename.c_str());
			ImGui::PopStyleColor();

			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImVec2 wp = ImGui::GetWindowPos();

			dot::color white = dot::color::white;
			ImColor color = ImColor(white.r, white.g, white.b, white.a);
			draw_list->AddCircleFilled(ImVec2(wp.x + 20, wp.y + 55), 12.0f, color, 64);
			ImGui::End();
		}

		ImGui::SetNextWindowPos(ImVec2(424, 0), ImGuiCond_Always, ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(600, 600), ImGuiCond_Always);

		ImGui::Begin("Preview", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);
		{
			float radius = 8.0f;
			float dradius = radius * 2.0f;
			float padding = 2.0f;

			dot::hardware& hw = _controller->get_hardware();
			auto& pixels = _controller->get_pixels();

			int rows = hw.rows;
			int colums = hw.colums;
			int width = hw.rows * (dradius + padding);
			int height = hw.colums * (dradius + padding);

			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImVec2 wp = ImGui::GetWindowPos();
			ImVec2 ws = ImGui::GetWindowSize();

			for (uint32_t j = 0; j < colums; ++j)
			{
				for (uint32_t i = 0; i < rows; ++i)
				{
					float x = (wp.x + dradius) + i * (dradius + padding) + ws.x * 0.5f - width * 0.5f;
					float y = (wp.y + dradius) + j * (dradius + padding) + ws.y * 0.5f - height * 0.5f;

					ImColor color_on = ImColor(0.8f, 0.8f, 0.8f);
					ImColor color_off = ImColor(0.3f, 0.3f, 0.3f);

					int idx = i + rows * j;
					draw_list->AddCircleFilled(ImVec2(x, y - 20), radius, pixels[idx] ? color_on : color_off, 64);
				}
			}

			ImGui::End();
		}
	}

private:
	std::vector<char> _buffer;
	dot::controller* _controller;
};