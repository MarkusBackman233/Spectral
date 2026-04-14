#ifdef EDITOR
#include "MaterialPropertyWindow.h"
#include "src/IMGUI/imgui_internal.h"
#include "StringUtils.h"
#include "DefaultMaterial.h"
#include "ResourceManager.h"
#include "IOManager.h"

MaterialPropertyWindow::MaterialPropertyWindow(std::function<void(std::shared_ptr<DefaultMaterial>)> onSelectedMaterial)
    : m_onSelectedMaterial(onSelectedMaterial)
{

}

void MaterialPropertyWindow::PopulateWindow()
{
    const ImVec2 buttonSize(ImGui::GetCurrentWindow()->Size.x, 30);
    ImGui::Separator();
    auto materials = ResourceManager::GetInstance()->GetResources<DefaultMaterial>();

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