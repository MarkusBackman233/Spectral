#ifdef EDITOR
#include "MeshPropertyWindow.h"
#include "src/IMGUI/imgui_internal.h"
#include "Editor.h"
#include "StringUtils.h"
#include "ModelManager.h"
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

    auto meshes = ModelManager::GetInstance()->GetCachedMeshes();

    for (const auto& [meshName, mesh] : meshes)
    {
        if (ImGui::Button(StringUtils::StripPathFromFilename(meshName).c_str(), buttonSize))
        {
            m_onSelectedMesh(mesh);
            CloseThisWindow();
            break;
        }
    }
}
#endif
