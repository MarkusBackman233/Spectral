#ifdef EDITOR
#include "ComponentPropertyWindow.h"
#include "src/IMGUI/imgui_internal.h"
#include "Editor.h"
#include "StringUtils.h"
#include "GameObject.h"
#include "ComponentFactory.h"
ComponentPropertyWindow::ComponentPropertyWindow(std::function<void(Component::Type)> onSelectedComponent)
    : m_onSelectedComponent(onSelectedComponent)
{

}

void ComponentPropertyWindow::PopulateWindow()
{
    const ImVec2 buttonSize(ImGui::GetCurrentWindow()->Size.x, 30);

    for (const auto& [componentId, componentName] : ComponentFactory::ComponentTypes)
    {
        if (ImGui::Button(componentName.c_str(), buttonSize))
        {
            m_onSelectedComponent(componentId);
            CloseThisWindow();
            break;
        }
    }
}

#endif