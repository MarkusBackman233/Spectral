#include "RenderManager.h"
#include "IOManager.h"
#include "TimeManager.h"
#include "ObjectManager.h"
#include "GameStateManager.h"
#include "InputManager.h"
#ifdef EDITOR
#include "Editor.h"
#include "EditorGUI.h"
#endif // EDITOR

#include <AudioManager.h>
#include <WindowsManager.h>


int main(int argc, char* args[])
{
    bool projectLoaded = argc > 1 ? IOManager::LoadProject(std::string(args[1])) : IOManager::LoadProject();

    if (projectLoaded == false)
    {
        return EXIT_SUCCESS;
    }

    GameStateManager stateManager;
    TimeManager timeManager;

#ifdef EDITOR
    //EditorGUI editor;
#endif


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
        //editor.Update();
        Editor::GetInstance()->Render();
#endif
        RenderManager::GetInstance()->Present();
    });

    AudioManager::GetInstance()->Deinitialize();
    ObjectManager::GetInstance()->Deinitialize();
    return EXIT_SUCCESS;
}
