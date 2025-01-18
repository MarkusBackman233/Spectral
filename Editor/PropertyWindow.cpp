#ifdef EDITOR
#include "PropertyWindow.h"
#include "src/IMGUI/imgui.h"
#include "Editor.h"

PropertyWindow::PropertyWindow()
    : m_windowOpen(true)
{
}

void PropertyWindow::Update()
{
    if (!ImGui::Begin("Select", &m_windowOpen, ImGuiWindowFlags_NoCollapse))
    {
        return;
    }
    if (m_windowOpen == false)
    {
        CloseThisWindow();
        ImGui::End();
        return;
    }

    PopulateWindow();
    ImGui::End();
}

void PropertyWindow::PopulateWindow()
{
    ImGui::Text("Not Implemented!");
}

void PropertyWindow::CloseThisWindow()
{
    m_windowOpen = false;
    Editor::GetInstance()->SetPropertyWindow(nullptr);
}
#endif // EDITOR
