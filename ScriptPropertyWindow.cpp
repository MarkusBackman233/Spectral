#ifdef EDITOR
#include "ScriptPropertyWindow.h"
#include "src/IMGUI/imgui_internal.h"
#include "StringUtils.h"
#include "ScriptManager.h"
#include "Script.h"
ScriptPropertyWindow::ScriptPropertyWindow(std::function<void(std::shared_ptr<Script>)> onSelectedScript)
    : m_onSelectedScript(onSelectedScript)
{

}

void ScriptPropertyWindow::PopulateWindow()
{
    const ImVec2 buttonSize(ImGui::GetCurrentWindow()->Size.x, 30);

    static bool open = true;
    if (ImGui::BeginPopupModal("New Script",&open))
    {
        static std::string ScriptName;
        auto cstrText = (char*)ScriptName.c_str();
        ImGui::SetNextItemWidth(200);
        if (ImGui::InputText("Script Name##ObjectName", cstrText, 255))
        {
        }
        ImGui::NewLine();
        if (ImGui::Button("Create", ImVec2(200,30)))
        {
            ScriptManager::GetInstance()->CreateScript(std::string(cstrText));
            ScriptName = "";
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (ImGui::Button("Create New Script", buttonSize))
    {
        open = true;
        ImGui::OpenPopup("New Script");
    }
    ImGui::Separator();
    for (const auto& [scriptName, Script] : ScriptManager::GetInstance()->GetScripts())
    {
        if (ImGui::Button(Script->GetFilename().c_str(), buttonSize))
        {
            m_onSelectedScript(Script);
            CloseThisWindow();
            break;
        }
    }
}
#endif