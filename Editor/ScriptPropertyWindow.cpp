#ifdef EDITOR
#include "ScriptPropertyWindow.h"
#include "src/IMGUI/imgui_internal.h"
#include "ResourceManager.h"
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
        char buffer[255] = {};
        strncpy_s(buffer, sizeof(buffer), ScriptName.c_str(), _TRUNCATE);
        buffer[sizeof(buffer) - 1] = '\0'; // ensure null-termination

        ImGui::SetNextItemWidth(200);
        if (ImGui::InputText("Script Name##ObjectName", buffer, sizeof(buffer))) {
            ScriptName = buffer;
            ScriptName.append(IOManager::GetResourceData<IOManager::ResourceType::Script>().SpectralExtension);
        }

        ImGui::NewLine();
        if (ImGui::Button("Create", ImVec2(200, 30))) {
            auto file = IOManager::ProjectDirectory /
                IOManager::GetResourceData<IOManager::ResourceType::Script>().Folder / ScriptName;

            std::filesystem::create_directories(file.parent_path());
            std::ofstream lua_file(file);

            lua_file << "\n\nfunction Start()\n\nend\n\nfunction Update(deltaTime)\n\nend\n";
            lua_file.close();

            auto script = ResourceManager::GetInstance()->GetResource<Script>(file);
            script->m_filename = ScriptName;
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
    auto scripts = ResourceManager::GetInstance()->GetResources<Script>();

    for (const auto& script : scripts)
    {
        if (ImGui::Button(script->GetFilename().c_str(), buttonSize))
        {
            m_onSelectedScript(script);
            CloseThisWindow();
            break;
        }
    }
}
#endif