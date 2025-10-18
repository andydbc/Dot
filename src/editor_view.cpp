#include "editor_view.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <string>
#include <fstream>

bool file_exist(const std::string& path)
{
	const std::ifstream file(path.c_str());
	return file.good();
}

void editor_view::initialize(dot::window& w)
{
	ImGuiIO& io = ImGui::GetIO();

	const std::string font("resources/fonts/input_mono_regular.ttf");
	if (file_exist(font))
	{
		io.Fonts->AddFontFromFileTTF(font.c_str(), 20.0f);
	}

	ImVec4 bg_color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
	w.set_clear_color(dot::color { bg_color.x, bg_color.y, bg_color.z, 1.0 });

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.89f, 1.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = bg_color;
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = bg_color;
	style.Colors[ImGuiCol_ChildBg] = bg_color;
	style.Colors[ImGuiCol_FrameBg] = bg_color;
	style.Colors[ImGuiCol_Button] = ImVec4(0.65f, 0.65f, 0.65f, 1.0f);
	style.ScrollbarRounding = 0;
	style.WindowBorderSize = 0;
	style.WindowRounding = 0;

	_has_changes = false;
}

float t = 0;

void editor_view::on_render(dot::window& w)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	{
		ImGui::SetNextWindowPos(ImVec2(35, 20), ImGuiCond_Always, ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(500, 450), ImGuiCond_Always);
		ImGui::Begin("TextEdit", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
		{
			_controller->get_script(_buffer);
			if(ImGui::InputTextMultiline("", &_buffer[0], _buffer.size(), ImVec2(500, 450), ImGuiInputTextFlags_AllowTabInput))
				_has_changes = true;
			ImGui::End();
			_controller->set_script(_buffer);
		}
	}

	ImGui::SetNextWindowPos(ImVec2(35, 525), ImGuiCond_Always, ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(400, 100), ImGuiCond_Always);
	ImGui::Begin("Info", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	{
		std::string filename = "Untitled";
		ImVec4 col = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);

		auto script_path = _controller->get_script_path();
		if (!script_path.empty())
		{
			const size_t last_slash_idx = script_path.find_last_of("\\/");
			if (std::string::npos != last_slash_idx)
			{
				script_path.erase(0, last_slash_idx + 1);
			}

			if (_has_changes)
			{
				script_path += '*';
			}

			col = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
			filename = script_path;
		}

		col = ImColor(0.89f, 1.0f, 0.0f, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_Text, col);
		ImGui::LabelText("", filename.c_str());
		ImGui::PopStyleColor();
		ImGui::End();
	}

	ImGui::SetNextWindowPos(ImVec2(550, 25), ImGuiCond_Always, ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(400, 550), ImGuiCond_Always);

	ImGui::Begin("Preview", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);
	{
		float radius = 8.0f;
		float dradius = radius * 2.0f;
		float padding = 2.0f;

		// dot::hardware& hw = _controller->get_hardware();
		auto& pixels = _controller->get_pixels();

		int rows = _controller->get_width();
		int colums = _controller->get_height();
		int width = rows * (dradius + padding);
		int height = colums * (dradius + padding);

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 wp = ImGui::GetWindowPos();
		ImVec2 ws = ImGui::GetWindowSize();

		for (uint32_t j = 0; j < colums; ++j)
		{
			for (uint32_t i = 0; i < rows; ++i)
			{
				float x = (wp.x + dradius) + i * (dradius + padding) + ws.x * 0.5f - width * 0.5f;
				float y = (wp.y + dradius) + j * (dradius + padding) + ws.y * 0.5f - height * 0.5f;

				ImColor color_on = ImColor(1.0f, 1.0f, 1.0f);
				ImColor color_off = ImColor(0.84f, 0.84f, 0.84f);

				int idx = i + rows * j;
				draw_list->AddCircleFilled(ImVec2(x, y - 20)
					, radius
					, pixels[idx] ? color_on : color_off
					, 32
				);
			}
		}

		ImGui::End();
	}

	ImGui::SetNextWindowPos(ImVec2(-10, 0), ImGuiCond_Always, ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(w.get_width(), 5), ImGuiCond_Always);
	
	ImGui::Begin("Timeline", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		ImVec2 wp = ImGui::GetWindowPos();
		ImVec2 ws = ImGui::GetWindowSize();

		int timeline = (((int)t % 100) / 100.0f) * w.get_width();
		
		auto col = ImColor(0.89f, 1.0f, 0.0f, 1.0f);
		draw_list->AddQuadFilled(wp
			, ImVec2(timeline, wp.y)
			, ImVec2(timeline, wp.y + 5)
			, ImVec2(wp.x, wp.y + 5)
			, col
		);

		t++;
		
		ImGui::End();
	}
}

void editor_view::on_save()
{
	_controller->save_script();
	_has_changes = false;
}