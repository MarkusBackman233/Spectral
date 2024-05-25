#include "Spectral.h"
#include "RenderManager.h"
#ifdef EDITOR
#include "Editor.h"
#endif // EDITOR
#include "IOManager.h"
#include "ProfilerManager.h"
#include "ProjectBrowserManager.h"
#include "DefaultAssets.h"
int main(int argc, char* args[])
{
    RenderManager::GetInstance();
    Editor::GetInstance();
    Spectral::GetInstance();

    IOManager::SetExecutableDirectiory();
    DefaultAssets::LoadDefaults();

#ifdef EDITOR
    RenderManager::GetInstance()->SetWindowIcon("icon.ico");
    {
        auto projectBrowser = ProjectBrowserManager();
        projectBrowser.StartBrowserWindow(); // Will wait here untill a project is selected
        RenderManager::GetInstance()->SetWindowTitle(std::string("Spectral | ") + IOManager::ProjectName);
    }
#endif

    IOManager::CollectProjectFiles();
    RenderManager::GetInstance()->SetWindowSize(Math::Vector2i(1280, 720));
    ShowWindow(RenderManager::GetInstance()->GetWindowHandle(), SW_SHOW);

    MSG msg{};
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0)
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