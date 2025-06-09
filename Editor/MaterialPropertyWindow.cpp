#ifdef EDITOR
#include "MaterialPropertyWindow.h"
#include "src/IMGUI/imgui_internal.h"
#include "StringUtils.h"
#include "Material.h"
#include "ResourceManager.h"
#include "IOManager.h"

MaterialPropertyWindow::MaterialPropertyWindow(std::function<void(std::shared_ptr<Material>)> onSelectedMaterial)
    : m_onSelectedMaterial(onSelectedMaterial)
{

}

void MaterialPropertyWindow::PopulateWindow()
{
    const ImVec2 buttonSize(ImGui::GetCurrentWindow()->Size.x, 30);


    if (ImGui::BeginPopupModal("New Material"))
    {
        static std::string materialName;
        char buffer[255] = {};
        strncpy_s(buffer, sizeof(buffer), materialName.c_str(), _TRUNCATE);
        buffer[sizeof(buffer) - 1] = '\0'; // ensure null-termination

        ImGui::SetNextItemWidth(200);
        if (ImGui::InputText("Script Name##ObjectName", buffer, sizeof(buffer))) {
            materialName = buffer;
        }
        if (ImGui::Button("Create", buttonSize))
        { 
            materialName.append(IOManager::GetResourceData<IOManager::ResourceType::Material>().SpectralExtension);

            auto material = std::make_shared<Material>();
            material->m_filename = materialName;
            material->SetTexture(0, ResourceManager::GetInstance()->GetResource<Texture>("TemplateGrid_albedo.bmp"));
            material->SetTexture(1, ResourceManager::GetInstance()->GetResource<Texture>("TemplateGrid_normal.bmp"));
            IOManager::SaveSpectralMaterial(material);

            auto file = IOManager::ProjectDirectory /
                IOManager::GetResourceData<IOManager::ResourceType::Material>().Folder / materialName;
            ResourceManager::GetInstance()->GetResource<Material>(file);
            materialName = "";
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (ImGui::Button("Create New Material", buttonSize))
    {
        ImGui::OpenPopup("New Material");
    }
    ImGui::Separator();
    auto materials = ResourceManager::GetInstance()->GetResources<Material>();

    for (const auto& material : materials)
    {
        if (ImGui::Button(StringUtils::StripPathFromFilename(material->GetFilename()).c_str(), buttonSize))
        {
            m_onSelectedMaterial(material);
            CloseThisWindow();
            break;
        }
    }
}
#endif