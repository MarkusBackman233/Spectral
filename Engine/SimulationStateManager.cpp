#include "SimulationStateManager.h"
#include "Editor.h"
#include "ObjectManager.h"
#include "iPhysics.h"
#include "iRender.h"
SimulationStateManager::SimulationStateManager()
    : m_wasRunning(false)
{
}

void SimulationStateManager::Update(float deltaTime)
{
    auto objectManager = ObjectManager::GetInstance();

    switch (GetSimulationState())
    {
    case SimulationState::Starting:
        objectManager->Start();
        break;
    case SimulationState::Resetting:
        objectManager->Reset();
        break;
    case SimulationState::Running:
        objectManager->Update(deltaTime);
        Physics::Simulate(deltaTime);
        if (GameObject* cameraObject = objectManager->GetMainCameraGameObject())
        {
            Render::GetCamera()->GetWorldMatrix() = cameraObject->GetWorldMatrix();
            Render::GetCamera()->GetWorldMatrix().OrthoNormalize();
            break;
        }
        break;
    case SimulationState::Stopped:
        break;
    }
}

SimulationState SimulationStateManager::GetSimulationState()
{
    SimulationState simulationState = SimulationState::Stopped;

#ifdef EDITOR
    bool isRunning = Editor::GetInstance()->IsStarted();
#else
    bool isRunning = true;
#endif
    if (m_wasRunning != isRunning)
    {
        m_wasRunning = isRunning;
        simulationState = isRunning ? SimulationState::Starting : SimulationState::Resetting;
    }
    else
    {
        simulationState = isRunning ? SimulationState::Running : SimulationState::Stopped;
    }

    return simulationState;
}
