#include "MaterialEditor.h"
#include "ResourceManager.h"
#include "DefaultMaterial.h"
#include "src/IMGUI/imgui.h"
#include "Editor.h"
#include "PropertyWindowFactory.h"
#include "Thumbnail.h"
void MaterialEditor::RenderGUI(std::shared_ptr<DefaultMaterial> material)
{
    ImGui::PushID(material.get());
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
                PropertyWindowFactory::SelectTexture(material, textureId, selectedTextureName);
                ThumbnailManager::RegenerateThumbnail(material.get());
            }
        }
        else
        {
            ImGui::Text(std::string(textureName + ": Not selected").c_str());

            if (ImGui::Button(std::string("##" + textureName).c_str(), Editor::GetInstance()->GetDefaultTextureSize()))
            {
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
    ImGui::ColorPicker4("Color", &material->GetMaterialSettings().Color.x, Editor::ColorPickerMask);

    if (isDisabled)
    {
        ImGui::EndDisabled();
    }
    ImGui::PopID();
}
