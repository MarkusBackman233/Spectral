#include "MaterialEditor.h"
#include "ResourceManager.h"
#include "DefaultMaterial.h"
#include "src/IMGUI/imgui.h"
void MaterialEditor::RenderGUI(std::shared_ptr<DefaultMaterial> material)
{

    bool isDisabled = material->GetFilename() == "Default.material";

    if (isDisabled)
    {
        ImGui::BeginDisabled();
    }

    enum TextureType
    {
        Albedo,
        Normal,
        Roughness,
        Metallic,
        AmbientOcclusion
    };

    static std::vector<std::pair<std::string, TextureType>> texturesNonCombined{
        {"Albedo", TextureType::Albedo},
        {"Normal", TextureType::Normal},
        {"Roughness", TextureType::Roughness},
        {"Metallic", TextureType::Metallic},
        {"Ao", TextureType::AmbientOcclusion},
    };
    static std::vector<std::pair<std::string, TextureType>> texturesCombined{
        {"Albedo", TextureType::Albedo},
        {"Normal", TextureType::Normal},
        {"MRO", TextureType::Roughness},
    };
    std::vector<std::pair<std::string, TextureType>>* textures = nullptr;


    if (material->GetMaterialSettings().CombinedMaterialTexture)
    {
        textures = &texturesCombined;
    }
    else
    {
        textures = &texturesNonCombined;
    }

    ImGui::Checkbox("Combined Material Texture", &material->GetMaterialSettings().CombinedMaterialTexture);

    for (const auto& [textureName, textureId] : *textures)
    {
        if (material->GetTexture(textureId) && material->GetTexture(textureId)->GetResourceView().Get())
        {
            auto selectedTextureName = material->GetTexture(textureId)->GetFilename();
            ImGui::Text(std::string(textureName + ": " + selectedTextureName).c_str());
            auto resource = material->GetTexture(textureId)->GetResourceView().Get();
            if (ImGui::ImageButton(textureName.c_str(), resource, Editor::GetInstance()->GetDefaultTextureSize()))
            {
                auto material = material;
                PropertyWindowFactory::SelectTexture(material, textureId, selectedTextureName);
            }
        }
        else
        {
            ImGui::Text(std::string(textureName + ": Not selected").c_str());

            if (ImGui::Button(std::string("##" + textureName).c_str(), Editor::GetInstance()->GetDefaultTextureSize()))
            {
                auto material = material;
                PropertyWindowFactory::SelectTexture(material, textureId);
            }
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSETS_BROWSER_ITEMS"))
            {
                size_t size = payload->DataSize / sizeof(FileItem);
                FileItem* payload_items = (FileItem*)payload->Data;
                for (size_t i = 0; i < size; i++)
                {
                    FileItem& item = payload_items[i];
                    if (item.m_type != ResourceType::Texture)
                    {
                        continue;
                    }
                    auto droppedTexture = ResourceManager::GetInstance()->GetResource<Texture>(item.m_filename);

                    if (droppedTexture)
                    {
                        material->SetTexture(textureId, droppedTexture);
                    }
                }




            }
            ImGui::EndDragDropTarget();
        }

        ImGui::Separator();
    }

    ImGui::DragFloat("Roughness", &material->GetMaterialSettings().Roughness, 0.05f, 0.0f, 1.0f);
    ImGui::DragFloat("Metallic", &material->GetMaterialSettings().Metallic, 0.05f, 0.0f, 1.0f);
    ImGui::Checkbox("Backface Culling", &material->GetMaterialSettings().BackfaceCulling);
    ImGui::Checkbox("Linear Filtering", &material->GetMaterialSettings().LinearFiltering);

    static constexpr int colorPickerMask = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoOptions 
        | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_NoTooltip
        | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoDragDrop
        | ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_Uint8;


    ImGui::ColorPicker4("Color", &material->GetMaterialSettings().Color.x, colorPickerMask);

    if (isDisabled)
    {
        ImGui::EndDisabled();
    }
}
