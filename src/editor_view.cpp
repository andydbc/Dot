#include "editor_view.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void editor_view::initialize(dot::window& w)
{
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("resources/fonts/input_mono_regular.ttf", 18.0f);

	ImVec4 bg_color = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
	w.set_clear_color(dot::color { bg_color.x, bg_color.y, bg_color.z, 1.0 });

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.45f, 0.45f, 0.45f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = bg_color;
	style.Colors[ImGuiCol_ChildBg] = bg_color;
	style.Colors[ImGuiCol_FrameBg] = bg_color;
	style.Colors[ImGuiCol_Button] = ImVec4(0.65f, 0.65f, 0.65f, 1.0f);
	style.ScrollbarRounding = 0;
	style.WindowBorderSize = 0;
	style.WindowRounding = 0;
}

void editor_view::on_render(dot::window& w)
{
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
		ImVec4 col = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);

		auto script_path = _controller->get_script_path();
		if (!script_path.empty())
		{
			const size_t last_slash_idx = script_path.find_last_of("\\/");
			if (std::string::npos != last_slash_idx)
			{
				script_path.erase(0, last_slash_idx + 1);
			}

			col = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
			filename = script_path;
		}

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

	ImGui::SetNextWindowPos(ImVec2(550, 0), ImGuiCond_Always, ImVec2(0, 0));
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
				ImColor color_off = ImColor(0.25f, 0.25f, 0.25f);

				int idx = i + rows * j;
				draw_list->AddCircleFilled(ImVec2(x, y - 20), radius, pixels[idx] ? color_on : color_off, 32);
			}
		}

		ImGui::End();
	}
}