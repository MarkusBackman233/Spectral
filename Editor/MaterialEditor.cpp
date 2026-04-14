#include "MaterialEditor.h"
#include "ResourceManager.h"
#include "DefaultMaterial.h"
#include "src/IMGUI/imgui.h"
#include "Editor.h"
#include "PropertyWindowFactory.h"
#include "Thumbnail.h"
bool MaterialEditor::RenderGUI(std::shared_ptr<DefaultMaterial> material)
{

    ImGui::SetWindowFontScale(1.4f);
    ImGui::SeparatorText(material->GetFilename().c_str());

    static char chars[255]{};

    if (ImGui::InputText("Name: ", chars, 255, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        material->m_filename = std::string(chars);
        try
        {
            auto newPath = material->GetPath().parent_path() / material->m_filename;
            newPath.replace_extension(material->GetPath().extension());
            std::filesystem::rename(material->GetPath(), newPath);
            material->SetPath(newPath);
        }
        catch (const std::exception&)
        {
            Logger::Error("Failed to rename file!");
        }


        Editor::GetInstance()->GetAssetBrowser()->RefreshFolder();
    }

    ImGui::SetWindowFontScale(1.0f);

    ImGui::PushID(material.get());
    bool isDisabled = material->GetFilename() == "Default.material";

    if (isDisabled)
    {
        ImGui::BeginDisabled();
    }

    bool changed = false;

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

    changed |= ImGui::Checkbox("Combined Material Texture", &material->GetMaterialSettings().CombinedMaterialTexture);

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
                changed |= true;

            }
        }
        else
        {
            ImGui::Text(std::string(textureName + ": Not selected").c_str());

            if (ImGui::Button(std::string("##" + textureName).c_str(), Editor::GetInstance()->GetDefaultTextureSize()))
            {
                PropertyWindowFactory::SelectTexture(material, textureId); 
                changed |= true;
            }
        }

        

        std::shared_ptr<Texture> droppedTexture;
        if (Editor::GetInstance()->GetDropResource(droppedTexture))
        {
            material->SetTexture(textureId, droppedTexture);
            changed |= true;
        }

        ImGui::Separator();
    }

    changed |= ImGui::DragFloat("Roughness", &material->GetMaterialSettings().Roughness, 0.05f, 0.0f, 1.0f);
    changed |= ImGui::DragFloat("Metallic", &material->GetMaterialSettings().Metallic, 0.05f, 0.0f, 1.0f);
    changed |= ImGui::DragFloat("TexelDensity", &material->GetMaterialSettings().TexelDensity, 0.05f, 0.0f, 100.0f);
    changed |= ImGui::Checkbox("Backface Culling", &material->GetMaterialSettings().BackfaceCulling);
    changed |= ImGui::Checkbox("Linear Filtering", &material->GetMaterialSettings().LinearFiltering);
    changed |= ImGui::ColorPicker4("Color", &material->GetMaterialSettings().Color.x, Editor::ColorPickerMask);

    if (isDisabled)
    {
        ImGui::EndDisabled();
    }

    if (changed)
    {
        ThumbnailManager::RegenerateThumbnail(material.get());
        material->SetUnsaved();
    }

    ImGui::PopID();

    return changed;
}
