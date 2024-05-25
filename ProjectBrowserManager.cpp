#include "ProjectBrowserManager.h"
#include "Editor.h"
#include "iRender.h"
#include "src/IMGUI/imgui.h"
#include "src/IMGUI/imgui_internal.h"
#include <ShlObj.h>
#include "IOManager.h"
#include <filesystem>
#include <tchar.h>
#include <codecvt>
#include "RenderManager.h"

void ProjectBrowserManager::StartBrowserWindow()
{
    m_previousProjectsFilename = std::string(IOManager::ExecutableDirectory + "PreviousProjects.txt");
    ReadPreviousProjects();

    if (!IsWindowVisible(RenderManager::GetInstance()->GetWindowHandle()))
    {
        ShowWindow(RenderManager::GetInstance()->GetWindowHandle(), SW_SHOW);
        RenderManager::GetInstance()->SetWindowSize(Math::Vector2i(720, 480));
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
            if (Update())
            {
                projectSelected = true;
            }
            Editor::GetInstance()->Render();
            RenderManager::GetInstance()->GetDeviceResources()->GetSwapChain()->Present(0, 0);
        }
        if (projectSelected)
        {
            break;
        }
    }
    ShowWindow(RenderManager::GetInstance()->GetWindowHandle(), SW_HIDE);

}
void ProjectBrowserManager::ReadPreviousProjects()
{
    m_previousProjects.clear();
    std::ifstream previousProjects(m_previousProjectsFilename);
    if (previousProjects) {
        std::string line;
        while (getline(previousProjects, line)) 
        {
            m_previousProjects.push_front(PreviousProject(line, ReadProjectNameFromIni(std::filesystem::path(line))));
        }
        previousProjects.close();
    }
    else {
        std::ofstream newfile(m_previousProjectsFilename);
        newfile.close();
    }
}


bool ProjectBrowserManager::Update()
{
    auto windowSize = Render::GetWindowSize();
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2((float)windowSize.x, (float)windowSize.y));

    auto windowFlags = ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoScrollbar;
    static bool openModal = false;

    bool projectSelected = false;

    if (ImGui::Begin("Project Browser", 0, windowFlags))
    {

        auto cursorPos = ImGui::GetCursorPos();
        auto availRegion = ImGui::GetContentRegionAvail();

        const float menuPadding = 5.0f;

        const float leftMenuSize = availRegion.x / 4;
        const float rightMenuSize = availRegion.x - availRegion.x / 4;

        ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_SpanAllColumns, menuPadding);
        ImGui::SetNextWindowPos(ImVec2(cursorPos.x, cursorPos.y + menuPadding));
        ImGui::SetNextWindowSize(ImVec2(leftMenuSize, availRegion.y - menuPadding));
        if (ImGui::Begin("##LeftSideCreate", 0, windowFlags))
        {
            ImVec2 buttonSize(leftMenuSize - menuPadding - menuPadding, 40.0f);
            ImGui::SetCursorPosX(ImGui::GetCursorPos().x + menuPadding - 1);
            if (ImGui::Button("New Project", buttonSize))
            {
                ImGui::OpenPopup("New Project");
            }

            const float modalPadding = 100.0f;
            ImGui::SetNextWindowPos(ImVec2(modalPadding * 0.5f, modalPadding * 0.5f));
            ImGui::SetNextWindowSize(ImVec2(availRegion.x - modalPadding, availRegion.y - modalPadding));
            bool openWindow = true;
            if (ImGui::BeginPopupModal("New Project", &openWindow, windowFlags)) 
            {

                static std::string projectName;
                char* cstrText = (char*)projectName.c_str();
                ImGui::SetCursorPosX(ImGui::GetCursorPos().x + menuPadding - 1);
                ImGui::Text("Project Name:");
                ImGui::SetCursorPosX(ImGui::GetCursorPos().x + menuPadding - 1);
                if (ImGui::InputText("##projectnameinput", cstrText, 255))
                {
                    projectName = std::string(cstrText);
                }
                ImGui::SetCursorPosX(ImGui::GetCursorPos().x + menuPadding - 1);
                ImGui::NewLine();
                static std::filesystem::path projectPath;
                char* cstrpath = (char*)projectPath.c_str();
                ImGui::SetCursorPosX(ImGui::GetCursorPos().x + menuPadding - 1);
                ImGui::Text("Path:");
                ImGui::SetCursorPosX(ImGui::GetCursorPos().x + menuPadding - 1);
                ImGui::InputText("##projectpathinput", cstrpath, 255);

                ImGui::SameLine();
                if (ImGui::Button("File Explorer"))
                {
                    projectPath = SelectFromFileExplorer();
                }

                ImGui::NewLine();
                ImGui::SetCursorPosX(ImGui::GetCursorPos().x + menuPadding - 1);
                bool simpleRender = false;
                ImGui::Checkbox("Simple render as default?", &simpleRender);
                ImGui::NewLine();

                ImGui::SetCursorPosX(ImGui::GetCursorPos().x + menuPadding - 1);
                if (ImGui::Button("Create", ImVec2(60, 0)))
                {
                    //if (!projectPath.string().empty() && !projectName.empty())
                    {
                        CreateProject(projectPath.parent_path().wstring(), projectName);

                        AddToPreviousProject(projectPath.string());
                        ReadPreviousProjects();

                        projectName = "";
                        projectPath = "";

                        ImGui::CloseCurrentPopup();
                    }
                }
                ImGui::EndPopup();
            }


            ImGui::SetNextWindowPos(ImVec2(modalPadding * 0.5f, modalPadding * 0.5f));
            ImGui::SetNextWindowSize(ImVec2(availRegion.x - modalPadding, availRegion.y - modalPadding));
            if (ImGui::BeginPopupModal("Project does not exist", &openWindow, windowFlags))
            {
                ImGui::NewLine();
                ImGui::Text("Folder does not have a Spectral_Project.ini");
                ImGui::NewLine();
                ImGui::NewLine();
                if (ImGui::Button("OK", buttonSize))
                {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            ImGui::SetCursorPosX(ImGui::GetCursorPos().x + menuPadding - 1);
            if (ImGui::Button("Load", buttonSize))
            {
                LoadProject();
            }
            ImGui::End();
        }
        ImGui::SetNextWindowPos(ImVec2(cursorPos.x + leftMenuSize, cursorPos.y + menuPadding));
        ImGui::SetNextWindowSize(ImVec2(availRegion.x, availRegion.y - menuPadding));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 1.0f, 1.0f, 0.06f));
        if (ImGui::Begin("Previous Projects", 0, windowFlags))
        {
            ImGui::Separator();

            ImGui::PopStyleColor();
            ImVec2 removeProjectbuttonSize(50 - menuPadding, 40.0f);
            ImVec2 projectButtonSize(rightMenuSize - menuPadding - menuPadding - menuPadding - 50.0f, 40.0f);
            for (const auto& projectPath : m_previousProjects)
            {
                ImGui::SetCursorPosX(ImGui::GetCursorPos().x + menuPadding);
                if (!projectPath.Name.empty() && ImGui::Button(projectPath.Name.c_str(), projectButtonSize))
                {
                    projectSelected = true;
                    IOManager::ProjectDirectory = projectPath.Path;
                    IOManager::ProjectDirectoryWide = std::filesystem::path(projectPath.Path).wstring();
                    IOManager::ProjectName = projectPath.Name;
                    RemoveProjectFromOldProjects(projectPath.Path);
                    m_previousProjects.push_front(PreviousProject(IOManager::ProjectDirectory, IOManager::ProjectName));
                    AddToPreviousProject(IOManager::ProjectDirectory);
                    break;
                }
                ImGui::SameLine();
                ImGui::PushID(projectPath.Path.c_str());
                if (ImGui::Button("X", removeProjectbuttonSize))
                {
                    RemoveProjectFromOldProjects(projectPath.Path);
                    ImGui::PopID();
                    break;
                }
                ImGui::PopID();
            }

            ImGui::End();
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
    
    std::ofstream outFile(m_previousProjectsFilename);
    outFile.close();

    for (const auto& projectPath : m_previousProjects)
    {
        AddToPreviousProject(projectPath.Path);
    }
}

void ProjectBrowserManager::CreateProject(const std::filesystem::path& path, const std::string& name)
{
    WCHAR iniFile[MAX_PATH];

    _stprintf_s(iniFile, _T("%s\\Spectral_Project.ini"), path.c_str());
     
    HANDLE hFile = CreateFile(iniFile, GENERIC_WRITE, FILE_SHARE_READ,
        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    CloseHandle(hFile);
    auto length = (int)name.length();
    size_t reqLength = ::MultiByteToWideChar(CP_UTF8, 0, name.c_str(), length, 0, 0);
    std::wstring ret(reqLength, L'\0');
    auto lengthRet = (int)ret.length();
    ::MultiByteToWideChar(CP_UTF8, 0, name.c_str(), length, &ret[0], lengthRet);
    
    BOOL writeResult = WritePrivateProfileString(L"Project", L"Name", ret.c_str(), iniFile);
    if (!writeResult) {
        // Handle error: Unable to write to the INI file
        DWORD error = GetLastError();
        std::cerr << "Error writing to INI file: " << error << std::endl;
    }
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
        if (alreadyInPrevious)
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
    return std::filesystem::exists(path.wstring() + L"\\Spectral_Project.ini");
}

std::string ProjectBrowserManager::ReadProjectNameFromIni(const std::filesystem::path& path)
{
    WCHAR iniFile[MAX_PATH];

    _stprintf_s(iniFile, _T("%s\\Spectral_Project.ini"), path.c_str());
    WCHAR buffer[256];
    GetPrivateProfileString(L"Project", L"Name", L"", buffer, 256, iniFile);

    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, nullptr, 0, nullptr, nullptr);
    if (bufferSize == 0) {
        // Handle error
        return "";
    }
    std::string result(bufferSize, 0);
    if (WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &result[0], bufferSize, nullptr, nullptr) == 0) {
        // Handle error
        return "";
    }
    return result;
}

std::filesystem::path ProjectBrowserManager::SelectFromFileExplorer()
{
    BROWSEINFO bi = { 0 };
    bi.lpszTitle = L"Select a folder";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    std::filesystem::path selectedPath;
    if (pidl != nullptr) {
        TCHAR path[MAX_PATH];
        if (SHGetPathFromIDList(pidl, path)) {
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
    previousProjectsFile.open(m_previousProjectsFilename, std::ios_base::app);
    previousProjectsFile << path.string() << "\n";
    previousProjectsFile.close();
}

