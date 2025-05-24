#include "GameStateManager.h"
#include "Editor.h"
#include "ObjectManager.h"
#include "iPhysics.h"
#include "iRender.h"
#include "NavigationManager.h"
#include "CameraComponent.h"

GameStateManager::GameStateManager()
    : m_gameState(GameState::Stopped)
{
}

void GameStateManager::Update(float deltaTime)
{
    UpdateGameState();

    auto objectManager = ObjectManager::GetInstance();
    auto navigationManager = NavigationManager::GetInstance();
  
    switch (m_gameState)
    {
    case GameState::Starting:
        navigationManager->GenerateNavMesh();
        objectManager->Start();
        break;
    case GameState::Resetting:
        objectManager->Reset();
        break;
    case GameState::Running:
        objectManager->Update(deltaTime);
        navigationManager->Update(deltaTime);
        Physics::Simulate(deltaTime);
        if (GameObject* cameraObject = objectManager->GetMainCameraGameObject())
        {
            Render::GetCamera()->GetWorldMatrix() = cameraObject->GetWorldMatrix();
            Render::GetCamera()->GetWorldMatrix().OrthoNormalize();
        }
        break;
    case GameState::Stopped:
        break;
    }
}

GameState GameStateManager::GetGameState() const
{
    return m_gameState;
}

void GameStateManager::UpdateGameState()
{ 
#ifdef EDITOR
    bool isRunning = Editor::GetInstance()->IsStarted();
#else
    bool isRunning = true;
#endif


    switch (m_gameState)
    {
    case GameState::Starting:
        m_gameState = GameState::Running;
        break;
    case GameState::Resetting:
        m_gameState = GameState::Stopped;
        break;
    case GameState::Running:
        if (!isRunning)
            m_gameState = GameState::Resetting;
        break;
    case GameState::Stopped:
        if (isRunning)
            m_gameState = GameState::Starting;
        break;
    }

}
