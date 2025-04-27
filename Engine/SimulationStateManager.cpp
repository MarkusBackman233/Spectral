#include "SimulationStateManager.h"
#include "Editor.h"
#include "ObjectManager.h"
#include "iPhysics.h"
#include "iRender.h"
#include "MathFunctions.h"
#include <BoxShape.h>
#include "AudioManager.h"

SimulationStateManager::SimulationStateManager()
    : m_wasRunning(false)
{
    //m_actor = Horizon::CreateDynamicActor(Horizon::Transform(Horizon::Vec3(0.0f, 5.0f, 0.0f)));
    //{
    //    auto shape = m_actor->AddShape(Horizon::ShapeType::Box);
    //    shape->SetLocalPose(Horizon::Transform(Horizon::Vec3(0.0f, 1.0f, 0.0f)));
    //    dynamic_cast<Horizon::BoxShape*>(shape)->SetHalfExtents(Horizon::Vec3(3,1,5));
    //}    
    //{
    //    auto shape = m_actor->AddShape(Horizon::ShapeType::Box);
    //    shape->SetLocalPose(Horizon::Transform(Horizon::Vec3(0.0f, -1.0f, 0.0f)));
    //    dynamic_cast<Horizon::BoxShape*>(shape)->SetHalfExtents(Horizon::Vec3(3, 1, 5));
    //}
    AudioManager::GetInstance();
}

void SimulationStateManager::Update(float deltaTime)
{
    auto objectManager = ObjectManager::GetInstance();
    /*
    for (auto& shape : m_actor->GetShapes())
    {

        auto pose = m_actor->GetPose() * shape->GetLocalPose();

        Math::Vector3 center(pose.pos.x, pose.pos.y, pose.pos.z);
        Horizon::Quat rotation = pose.quat;
        Horizon::Vec3 halfExtents = dynamic_cast<Horizon::BoxShape*>(shape)->GetHalfExtents();
        float h = 0.5f;
        Math::Vector3 localCorners[8] = {
            Math::Vector3(-halfExtents.x, -halfExtents.y, -halfExtents.z),
            Math::Vector3(halfExtents.x, -halfExtents.y, -halfExtents.z),
            Math::Vector3(halfExtents.x,  halfExtents.y, -halfExtents.z),
            Math::Vector3(-halfExtents.x,  halfExtents.y, -halfExtents.z),
            Math::Vector3(-halfExtents.x, -halfExtents.y,  halfExtents.z),
            Math::Vector3(halfExtents.x, -halfExtents.y,  halfExtents.z),
            Math::Vector3(halfExtents.x,  halfExtents.y,  halfExtents.z),
            Math::Vector3(-halfExtents.x,  halfExtents.y,  halfExtents.z)
        };

        Math::Vector3 worldCorners[8];
        for (int i = 0; i < 8; ++i)
        {
            Horizon::Vec3 rotated = rotation.RotateVector( Horizon::Vec3(localCorners[i].x, localCorners[i].y, localCorners[i].z));
            worldCorners[i] = center + Math::Vector3(rotated.x, rotated.y, rotated.z);
        }

        // Draw edges
        Render::DrawLine(worldCorners[0], worldCorners[1]);
        Render::DrawLine(worldCorners[1], worldCorners[2]);
        Render::DrawLine(worldCorners[2], worldCorners[3]);
        Render::DrawLine(worldCorners[3], worldCorners[0]);
        Render::DrawLine(worldCorners[4], worldCorners[5]);
        Render::DrawLine(worldCorners[5], worldCorners[6]);
        Render::DrawLine(worldCorners[6], worldCorners[7]);
        Render::DrawLine(worldCorners[7], worldCorners[4]);
        Render::DrawLine(worldCorners[0], worldCorners[4]);
        Render::DrawLine(worldCorners[1], worldCorners[5]);
        Render::DrawLine(worldCorners[2], worldCorners[6]);
        Render::DrawLine(worldCorners[3], worldCorners[7]);
    }

    */
    switch (GetSimulationState())
    {
    case SimulationState::Starting:
        objectManager->Start();
        break;
    case SimulationState::Resetting:
        objectManager->Reset();
        //m_actor->SetPose(Horizon::Transform(Horizon::Vec3(0.0f, 5.0f, 0.0f)));
        //m_actor->Reset();
        break;
    case SimulationState::Running:
        objectManager->Update(deltaTime);
        Physics::Simulate(deltaTime);
        Horizon::Simulate(deltaTime);
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
