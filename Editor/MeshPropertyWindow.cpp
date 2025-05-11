#ifdef EDITOR
#include "MeshPropertyWindow.h"
#include "src/IMGUI/imgui_internal.h"
#include "Editor.h"
#include "StringUtils.h"
#include "ResourceManager.h"
#include "MeshComponent.h"
#include "GameObject.h"
#include "Mesh.h"

MeshPropertyWindow::MeshPropertyWindow(std::function<void(std::shared_ptr<Mesh>)> onSelectedMesh)
{
    m_onSelectedMesh = onSelectedMesh;
}

void MeshPropertyWindow::PopulateWindow()
{
    const ImVec2 buttonSize(ImGui::GetCurrentWindow()->Size.x, 30);

    auto meshes = ResourceManager::GetInstance()->GetResources<Mesh>();

    for (const auto& mesh: meshes)
    {
        if (ImGui::Button(mesh->GetFilename().c_str(), buttonSize))
        {
            m_onSelectedMesh(mesh);
            CloseThisWindow();
            break;
        }
    }
}
#endif
