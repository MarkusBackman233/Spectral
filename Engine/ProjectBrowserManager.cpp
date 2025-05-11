#ifdef EDITOR
#include "ProjectBrowserManager.h"
#include "Editor.h"
#include "iRender.h"
#include "src/IMGUI/imgui.h"
#include "src/IMGUI/imgui_internal.h"
#include <ShlObj.h>
#include "IOManager.h"
#include "RenderManager.h"
#include "StringUtils.h"
#include "Json.h"

ProjectBrowserManager::ProjectBrowserManager()
    : m_hasLoadedProject(false)
{
    Render::SetWindowIcon("icon.ico");
    Render::SetWindowTitle("Spectral | Project Browser");
    m_previousProjectsPath = IOManager::ExecutableDirectory / "PreviousProjects.txt";
    ReadPreviousProjects();

    if (!IsWindowVisible(Render::GetWindowHandle()))
    {
        Render::ShowWindow(true);
        Render::SetWindowSize(Math::Vector2i(720, 480));
    }

    MSG msg{};
    msg.message = WM_NULL;
    PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);
    bool projectSelected = false;

    while (WM_QUIT != msg.message)
    {
        bool gotMessage = (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0);
        if (gotMessage)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            RenderManager::GetInstance()->Render();
            Editor::GetInstance()->PreRender();
            projectSelected = Update();
            Editor::GetInstance()->Render();
            RenderManager::GetInstance()->Present();
        }
        if (projectSelected)
        {
            break;
        }
    }

    if (msg.message == WM_QUIT)
    {
        m_hasLoadedProject = false;
        return;
    }

    Render::ShowWindow(false);
    Render::SetWindowTitle(std::string("Spectral | ") + IOManager::ProjectName);
    IOManager::CollectProjectFiles();

    auto projectDescription = Json::ParseFile(IOManager::ProjectDirectory / "Project_Description.json");
    if (projectDescription.Has("LastScene"))
    {
        IOManager::LoadSpectralScene(projectDescription["LastScene"].AsString());
    }

    m_hasLoadedProject = true;
    return;
}

bool ProjectBrowserManager::HasLoadedProject()
{
    return m_hasLoadedProject;
}

void ProjectBrowserManager::ReadPreviousProjects()
{
    m_previousProjects.clear();
    std::ifstream previousProjects(m_previousProjectsPath);
    if (previousProjects) 
    {
        std::string line;
        while (getline(previousProjects, line)) 
        {
            if(DoesProjectFileExist(std::filesystem::path(line)))
            {
                m_previousProjects.push_front(PreviousProject(line, ReadProjectNameFromIni(std::filesystem::path(line))));
            }
        }
        previousProjects.close();
    }
    else 
    {
        std::ofstream newfile(m_previousProjectsPath);
        newfile.close();
    }
}


bool ProjectBrowserManager::Update()
{
    auto windowSize = Render::GetWindowSize();
    if (windowSize.x == 0 || windowSize.y == 0)
    {
        return false;
    }

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2((float)windowSize.x, (float)windowSize.y));

    auto windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar;
    static bool openModal = false;
    bool projectSelected = false;
    
    if (ImGui::Begin("Project Browser", nullptr, windowFlags | ImGuiWindowFlags_NoTitleBar))
    {
        auto cursorPos = ImGui::GetCursorPos();
        auto availRegion = ImGui::GetContentRegionAvail();

        const float menuPadding = 5.0f;

        const float leftMenuSize = availRegion.x / 4;
        const float rightMenuSize = availRegion.x - leftMenuSize;
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_SpanAllColumns, menuPadding);
        //ImGui::SetNextWindowPos(ImVec2(cursorPos.x, cursorPos.y + menuPadding));
        if(ImGui::BeginChild("meme##LeftSideCreate", ImVec2(leftMenuSize, ImGui::GetContentRegionAvail().y),true))
        {
            ImGui::NewLine();
            ImVec2 buttonSize(ImGui::GetContentRegionAvail().x, 40.0f);
            if (ImGui::Button("New Project", buttonSize))
            {
                ImGui::OpenPopup("New Project");
            }

            ImGui::SetNextWindowPos(ImVec2(availRegion.x * 0.5f - 154.0f, availRegion.y * 0.5f - 62.0f));
            ImGui::SetNextWindowSize(ImVec2(308.0f, 184.0f));
            bool openWindow = true;
            if (ImGui::BeginPopupModal("New Project", &openWindow, windowFlags)) 
            {
                static std::filesystem::path projectPath;
                static std::string projectName;


                if (ImGui::Button("File Explorer"))
                {
                    projectPath = SelectFromFileExplorer();
                }


                ImGui::Text("Project Name:");
                char nameBuffer[256] = { 0 };
                strncpy_s(nameBuffer, projectName.c_str(), sizeof(nameBuffer));
                ImGui::InputText("##projectnameinput", nameBuffer, sizeof(nameBuffer));
                projectName = nameBuffer;

                ImGui::Separator();

                ImGui::TextWrapped((projectPath / projectName).string().c_str());

                if (projectPath.empty()|| projectName.empty())
                {
                    ImGui::BeginDisabled();
                }
                if (ImGui::Button("Create", ImVec2(60, 0)))
                {
                    projectPath.append(projectName);
                    CreateProject(projectPath, projectName);

                    AddToPreviousProject((projectPath).string());
                    ReadPreviousProjects();

                    projectName = "";
                    projectPath = "";

                    ImGui::CloseCurrentPopup();
                }
                else if (projectPath.empty() || projectName.empty())
                {
                    ImGui::EndDisabled();
                }

                ImGui::EndPopup();
            }
            bool openWindow2 = true;

            ImGui::SetNextWindowPos(ImVec2(availRegion.x * 0.5f - 154.0f, availRegion.y * 0.5f - 62.0f));
            ImGui::SetNextWindowSize(ImVec2(308.0f, 124.0f));
            if (ImGui::BeginPopupModal("Project does not exist", &openWindow2, windowFlags))
            {
                ImGui::NewLine();
                ImGui::Text("Folder does not have a Project_Description.json");
                ImGui::NewLine();
                ImGui::NewLine();
                if (ImGui::Button("OK", buttonSize))
                {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            if (ImGui::Button("Load", buttonSize))
            {
                LoadProject();
            }
            ImGui::EndChild();
        }
        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        if (ImGui::BeginChild("meme##asdasd", ImGui::GetContentRegionAvail(), false))
        {
            ImGui::Text("Previous Projects");

            for (const auto& [path, name] : m_previousProjects)
            {
                if (name.size() < 3)
                {
                    continue;
                }
                if (ImGui::Button(name.c_str(), ImVec2(ImGui::GetContentRegionAvail().x - 50, 40.0f)))
                {
                    projectSelected = true;
                    SelectProject(path, name);
                    break;
                }
                ImGui::SameLine();
                ImGui::PushID(path.c_str());
                if (ImGui::Button("X", ImVec2(ImGui::GetContentRegionAvail().x, 40.0f)))
                {
                    RemoveProjectFromOldProjects(path);
                    ImGui::PopID();
                    break;
                }
                ImGui::PopID();
            }

            ImGui::EndChild();
        }


        ImGui::End();
    }

    return projectSelected;
}

void ProjectBrowserManager::RemoveProjectFromOldProjects(const std::filesystem::path& path)
{
    for (auto it = m_previousProjects.begin(); it < m_previousProjects.end(); it++)
    {
        if (it->Path == path)
        {
            m_previousProjects.erase(it);
            break;
        }
    }
    
    std::ofstream outFile(m_previousProjectsPath);
    outFile.close();

    for (const auto& projectPath : m_previousProjects)
    {
        AddToPreviousProject(projectPath.Path);
    }
}

void ProjectBrowserManager::CreateProject(const std::filesystem::path& path, const std::string& name)
{

    std::error_code ec;
    std::filesystem::create_directories(path, ec); // Creates all intermediate directories if needed
    if (ec) {
        Logger::Error("Failed to create project directory: " + ec.message());
        return;
    }

    std::filesystem::path filePath = path / L"Project_Description.json";
    Json::Object project;
    project.emplace("Name", name);
    Json::Serialize(project, filePath);

    //HANDLE hFile = CreateFile(iniFile.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    //CloseHandle(hFile);
    //
    //BOOL writeResult = WritePrivateProfileString(L"Project", L"Name", StringUtils::StringToWideString(name).c_str(), iniFile.c_str());
    //if (!writeResult) 
    //{
    //    Logger::Error("Error writing to INI file: " + GetLastError());
    //}
}

void ProjectBrowserManager::SelectProject(const std::filesystem::path& path, const std::string& name)
{
    IOManager::ProjectDirectory = path;
    IOManager::ProjectName = name;
    RemoveProjectFromOldProjects(path);
    m_previousProjects.push_front(PreviousProject(IOManager::ProjectDirectory, IOManager::ProjectName));
    AddToPreviousProject(IOManager::ProjectDirectory);
}

void ProjectBrowserManager::LoadProject()
{
    auto path = SelectFromFileExplorer();

    if (DoesProjectFileExist(path))
    {
        auto name = ReadProjectNameFromIni(path);


        bool alreadyInPrevious = false;

        for (const auto& project : m_previousProjects)
        {
            if (project.Name == name)
            {
                alreadyInPrevious = true;
                break;
            }
        }
        if (alreadyInPrevious) // if the project already exists, remove it and add it to push it to the front.
        {
            RemoveProjectFromOldProjects(path);
        }

        m_previousProjects.push_front(PreviousProject(path.string(), name));
        AddToPreviousProject(path);
    }
    else
    {
        ImGui::OpenPopup("Project does not exist");
    }
}

bool ProjectBrowserManager::DoesProjectFileExist(const std::filesystem::path& path)
{
    return std::filesystem::exists(path / L"Project_Description.json");
}


std::string ProjectBrowserManager::ReadProjectNameFromIni(const std::filesystem::path& path)
{

    Json projectDescription = Json::ParseFile(path / "Project_Description.json");
    if (projectDescription.HasError())
    {
        return "";
    }

    return projectDescription["Name"].AsString();
}

std::filesystem::path ProjectBrowserManager::SelectFromFileExplorer()
{
    BROWSEINFO bi = { 0 };
    bi.lpszTitle = L"Select a folder";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    std::filesystem::path selectedPath;
    if (pidl != nullptr) 
    {
        TCHAR path[MAX_PATH];
        if (SHGetPathFromIDList(pidl, path)) 
        {
            selectedPath = path;
            selectedPath.append("\0");
        }

        CoTaskMemFree(pidl);
    }
    return selectedPath;
}

void ProjectBrowserManager::AddToPreviousProject(const std::filesystem::path& path) const
{
    std::ofstream previousProjectsFile;
    previousProjectsFile.open(m_previousProjectsPath, std::ios_base::app);
    previousProjectsFile << path.string() << "\n";
    previousProjectsFile.close();
}
#endif