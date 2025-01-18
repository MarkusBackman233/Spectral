#include "RenderManager.h"
#include "IOManager.h"
#include "TimeManager.h"
#include "ObjectManager.h"
#include "SimulationStateManager.h"
#include "InputManager.h"
#ifdef EDITOR
#include "Editor.h"
#endif // EDITOR

int main(int argc, char* args[])
{
    if (IOManager::LoadProject() == false)
    {
        return EXIT_SUCCESS;
    }

    SimulationStateManager stateManager;
    TimeManager timeManager;

    WindowsManager::CreateWindowsLoop(
        [&]
    {
        timeManager.Update();
        InputManager::GetInstance()->Update();
        stateManager.Update(timeManager.GetDeltaTime());
        ObjectManager::GetInstance()->Render();
        RenderManager::GetInstance()->Render();
#ifdef EDITOR
        Editor::GetInstance()->Update(timeManager.GetDeltaTime());
        Editor::GetInstance()->Render();
#endif
        RenderManager::GetInstance()->Present();
    });

    ObjectManager::GetInstance()->Deinitialize();
    return EXIT_SUCCESS;
}
