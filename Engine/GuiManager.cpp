#include "GuiManager.h"
#include "src/IMGUI/imgui.h"
#include "iRender.h"
#include "src/IMGUI/imgui_impl_win32.h"
#include "src/IMGUI/imgui_impl_dx11.h"
GuiManager::GuiManager()
{

}

void GuiManager::CreateResources(ID3D11Device* device, HWND hwnd, ID3D11DeviceContext* context)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(device, context);
}

void GuiManager::Text(const std::string& text, const Math::Vector2& position)
{
	m_textToDraw.emplace_back(text, position);
}

void GuiManager::Render()
{
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2((float)Render::GetWindowSize().x, (float)Render::GetWindowSize().y));
	ImGui::Begin("GuiLayer",0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs);

	for (const auto& [text, position] : m_textToDraw)
	{
		ImGui::SetCursorPos(ImVec2(position.x, position.y));
		ImGui::Text(text.c_str());
	}

	ImGui::End();
	m_textToDraw.clear();
}

