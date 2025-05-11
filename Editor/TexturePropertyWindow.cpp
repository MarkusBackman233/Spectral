#ifdef EDITOR
#include "TexturePropertyWindow.h"
#include "src/IMGUI/imgui_internal.h"
#include "Editor.h"
#include "StringUtils.h"
#include "GameObject.h"
#include "Texture.h"
#include "IOManager.h"
#include "ResourceManager.h"

TexturePropertyWindow::TexturePropertyWindow(std::function<void(std::shared_ptr<Texture>)> onSelectedTexture, const std::string& currentSelectedTextureName)
    : m_currentSelectedTextureName(currentSelectedTextureName)
    , m_onSelectedTexture(onSelectedTexture)
{
}

void TexturePropertyWindow::PopulateWindow()
{
    auto imageSize = ImVec2(100, 100);
    int columnsPerRow = std::clamp((int)std::floor(ImGui::GetCurrentWindow()->Size.x / (imageSize.x + 15)), 1, 20);
    static char searchedString[128] = "";


    ImGui::PushItemWidth(300);
    ImGui::InputText("Search", searchedString, IM_ARRAYSIZE(searchedString));
    ImGui::PopItemWidth();
    ImGui::Separator();
    if (ImGui::Button("None"))
    {
        m_onSelectedTexture(nullptr);
        CloseThisWindow();
        return;
    }
    if (ImGui::BeginTable("##TextureSelector", columnsPerRow, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingFixedFit))
    {

        auto textures = ResourceManager::GetInstance()->GetResources<Texture>();

        for (auto& texture : textures)
        {
            if (!StringUtils::StringContainsCaseInsensitive(texture->m_filename, searchedString))
            {
                continue;
            }

            auto bgColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            if (m_currentSelectedTextureName == StringUtils::StripPathFromFilename(texture->m_filename))
            {
                bgColor = ImVec4(1.0f, 0.3f, 1.0f, 1.0f);
            }

            ImGui::TableNextColumn();
            if (ImGui::ImageButton(texture->m_filename.c_str(), texture->GetResourceView().Get(), imageSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), bgColor))
            {
                m_onSelectedTexture(texture);
                CloseThisWindow();
                ImGui::EndTable();
                return;
            }


            ImGui::TextWrapped(texture->m_filename.c_str());
        }
        ImGui::EndTable();
    }
}
#endif