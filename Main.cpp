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

int main(int argc, char* args[])
{
    RenderManager::GetInstance();
    Editor::GetInstance();
    Spectral::GetInstance();

#ifdef _RELEASE

    WCHAR path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);

    std::filesystem::path currentDir = std::wstring(path);

    std::filesystem::path parentDir = currentDir.parent_path();

    IOManager::ProjectDirectoryWide = std::wstring(std::wstring(parentDir.wstring()) + L"\\Exported\\");

    char* buffer = new char[IOManager::ProjectDirectoryWide.size()];

    WideCharToMultiByte(CP_UTF8, 0, IOManager::ProjectDirectoryWide.c_str(), -1, buffer, IOManager::ProjectDirectoryWide.size(), NULL, NULL);

    std::string normalStr(buffer);
    delete[] buffer;
    IOManager::ProjectDirectory = normalStr;
#endif

    IOManager::CollectProjectFiles();


    if (!IsWindowVisible(RenderManager::GetInstance()->GetWindowHandle()))
        ShowWindow(RenderManager::GetInstance()->GetWindowHandle(), SW_SHOW);
    if (FAILED(OleInitialize(nullptr))) {
        // Handle initialization failure
    }
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

            {
                auto timer = ProfilerManager::GetInstance()->GetProfileObject("All updates");
                timer.StartTimer();
                Spectral::GetInstance()->Update();
                RenderManager::GetInstance()->Update();
                Spectral::GetInstance()->Render();
            }
            RenderManager::GetInstance()->Render();
#ifdef EDITOR
            {
                auto timer = ProfilerManager::GetInstance()->GetProfileObject("Editor");
                timer.StartTimer();
                Editor::GetInstance()->Update();
                Editor::GetInstance()->Render();
            }
#endif
            {
                auto timer = ProfilerManager::GetInstance()->GetProfileObject("Present");
                timer.StartTimer();
                RenderManager::GetInstance()->GetDeviceResources()->GetSwapChain()->Present(0, 0);
            }
        }
    }

    return EXIT_SUCCESS;
}