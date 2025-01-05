#ifdef EDITOR
#include "MaterialPropertyWindow.h"
#include "src/IMGUI/imgui_internal.h"
#include "StringUtils.h"
#include "Material.h"
#include "MaterialManager.h"

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
        auto cstrText = (char*)materialName.c_str();
        if (ImGui::InputText("Material Name##ObjectName", cstrText, 255))
        {
        }
        if (ImGui::Button("Create", buttonSize))
        {
            MaterialManager::GetInstance()->GetMaterial(std::string(cstrText));
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
    for (const auto& [materialName, material] : MaterialManager::GetInstance()->GetMaterials())
    {
        if (ImGui::Button(StringUtils::StripPathFromFilename(material->GetName()).c_str(), buttonSize))
        {
            m_onSelectedMaterial(material);
            CloseThisWindow();
            break;
        }
    }
}
#endif