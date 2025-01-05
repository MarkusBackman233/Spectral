#ifdef EDITOR
#include "TexturePropertyWindow.h"
#include "src/IMGUI/imgui_internal.h"
#include "Editor.h"
#include "StringUtils.h"
#include "GameObject.h"
#include "Texture.h"
#include "TextureManager.h"
#include "IOManager.h"

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
        auto textures = TextureManager::GetInstance()->GetCachedTextures();

        for (const auto& [textureName, texture] : textures)
        {
            if (!StringUtils::StringContainsCaseInsensitive(textureName, searchedString))
            {
                continue;
            }

            auto bgColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            if (m_currentSelectedTextureName == StringUtils::StripPathFromFilename(textureName))
            {
                bgColor = ImVec4(1.0f, 0.3f, 1.0f, 1.0f);
            }

            ImGui::TableNextColumn();
            if (ImGui::ImageButton(textureName.c_str(), texture->GetResourceView().Get(), imageSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), bgColor))
            {
                m_onSelectedTexture(texture);
                CloseThisWindow();
                ImGui::EndTable();
                return;
            }


            ImGui::TextWrapped(textureName.c_str());
        }
        ImGui::EndTable();
    }
}
#endif