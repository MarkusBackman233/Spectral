#include "Spectral.h"
#include "RenderManager.h"
#ifdef EDITOR
#include "Editor.h"
#endif // EDITOR
#include "IOManager.h"
#include "ProfilerManager.h"
#include <locale>
#include <codecvt>
#include "filesystem"
#include "ProjectBrowserManager.h"
#include <tchar.h>
#include "DefaultAssets.h"
int main(int argc, char* args[])
{
    RenderManager::GetInstance();
    Editor::GetInstance();
    Spectral::GetInstance();
    WCHAR path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);

    std::filesystem::path currentDir = std::wstring(path);
    std::filesystem::path parentDir = currentDir.parent_path();
    IOManager::ExecutableDirectory = currentDir.parent_path().string() + std::string("\\");
    IOManager::ExecutableDirectoryWide = currentDir.parent_path().wstring() + std::wstring(L"\\");
    
    DefaultAssets::LoadDefaults();


#ifdef EDITOR
    ProjectBrowserManager::GetInstance();
    HANDLE hIcon = LoadImage(0, _T("icon.ico"), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
    SendMessage(RenderManager::GetInstance()->GetWindowHandle(), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    SendMessage(RenderManager::GetInstance()->GetWindowHandle(), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    {
        if (!IsWindowVisible(RenderManager::GetInstance()->GetWindowHandle()))
        {
            ShowWindow(RenderManager::GetInstance()->GetWindowHandle(), SW_SHOW);

            RECT desktop;
            // Get a handle to the desktop window
            const HWND hDesktop = GetDesktopWindow();
            // Get the size of screen to the variable desktop
            GetWindowRect(hDesktop, &desktop);
            ::SetWindowLong(RenderManager::GetInstance()->GetWindowHandle(), GWL_STYLE, GetWindowLong(RenderManager::GetInstance()->GetWindowHandle(), GWL_STYLE) & ~WS_SIZEBOX);

            const int projectBrowserWindowSizeWidth = 720;
            const int projectBrowserWindowSizeHeight = 480;

            MoveWindow(RenderManager::GetInstance()->GetWindowHandle(), 
                desktop.right / 2 - projectBrowserWindowSizeWidth / 2,
                desktop.bottom / 2 - projectBrowserWindowSizeHeight / 2, projectBrowserWindowSizeWidth,
                projectBrowserWindowSizeHeight, TRUE);
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
                if (ProjectBrowserManager::GetInstance()->Update())
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
        if (projectSelected == false)
        {
            return 1;
        }
    }
#endif

    ShowWindow(RenderManager::GetInstance()->GetWindowHandle(), SW_HIDE);
    IOManager::CollectProjectFiles();
    RECT desktop;
    // Get a handle to the desktop window
    const HWND hDesktop = GetDesktopWindow();
    // Get the size of screen to the variable desktop
    GetWindowRect(hDesktop, &desktop);

    const int editorWindowSizeWidth = 1280;
    const int editorWindowSizeHeight = 720;

    MoveWindow(RenderManager::GetInstance()->GetWindowHandle(),
        desktop.right / 2 - editorWindowSizeWidth / 2,
        desktop.bottom / 2 - editorWindowSizeHeight / 2, editorWindowSizeWidth,
        editorWindowSizeHeight, TRUE);
    SetWindowTextA(RenderManager::GetInstance()->GetWindowHandle(), std::string(std::string("Spectral | ") + IOManager::ProjectName).c_str());
    ShowWindow(RenderManager::GetInstance()->GetWindowHandle(), SW_SHOW);

    MSG msg{};
    msg.message = WM_NULL;
    PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

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

            Spectral::GetInstance()->Update();
            RenderManager::GetInstance()->Update();
            Spectral::GetInstance()->Render();
            RenderManager::GetInstance()->Render();
#ifdef EDITOR

            Editor::GetInstance()->Update();
            Editor::GetInstance()->Render();
#endif
            RenderManager::GetInstance()->GetDeviceResources()->GetSwapChain()->Present(0, 0);
        }
    }

    return EXIT_SUCCESS;
}