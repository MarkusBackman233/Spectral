#include "SimulationStateManager.h"
#include "Editor.h"
#include "ObjectManager.h"
#include "iPhysics.h"
#include "iRender.h"
#include "NavigationManager.h"

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
        NavigationManager::GetInstance()->GenerateNavMesh();
        objectManager->Start();
        break;
    case SimulationState::Resetting:
        objectManager->Reset();
        break;
    case SimulationState::Running:
        objectManager->Update(deltaTime);
        NavigationManager::GetInstance()->m_crowd->update(deltaTime, nullptr);
        Physics::Simulate(deltaTime);
        
        if (GameObject* cameraObject = objectManager->GetMainCameraGameObject())
        {
            Render::GetCamera()->GetWorldMatrix() = cameraObject->GetWorldMatrix();
            Render::GetCamera()->GetWorldMatrix().OrthoNormalize();
        }

        {
            const auto navMesh = NavigationManager::GetInstance()->m_navMesh;
            for(int i = 0; i < navMesh->getMaxTiles(); ++i)
            {
                const dtMeshTile* tile = navMesh->getTileG(i);

                const dtPoly* polys = tile->polys;
                const float* verts = tile->verts;
                const dtPolyDetail* detailMeshes = tile->detailMeshes;
                const float* detailVerts = tile->detailVerts;

                for (int j = 0; j < tile->header->polyCount; ++j)
                {
                    const dtPoly& p = polys[j];
                    if (p.getType() != DT_POLYTYPE_GROUND) continue; // skip off-mesh connections

                    const dtPolyDetail& pd = detailMeshes[j];

                    for (int k = 0; k < pd.triCount; ++k)
                    {
                        const unsigned char* t = &tile->detailTris[(pd.triBase + k) * 4];

                        Math::Vector3 tri[3];
                        for (int m = 0; m < 3; ++m)
                        {
                            const int vertIndex = t[m];
                            const float* v;

                            if (vertIndex < pd.vertCount)
                                v = &detailVerts[(pd.vertBase + vertIndex) * 3];
                            else
                                v = &verts[p.verts[vertIndex - pd.vertCount] * 3];

                            tri[m] = Math::Vector3(v[0], v[1], v[2]);
                        }

                        // Draw triangle as lines
                        Render::DrawLine(tri[0], tri[1]);
                        Render::DrawLine(tri[1], tri[2]);
                        Render::DrawLine(tri[2], tri[0]);
                    }
                }
            }

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
