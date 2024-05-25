#include "Spectral.h"
#include <iostream>
#include <algorithm>
#include "Math.h"
#include <vector>
#include "Vector3.h"
#include "Mesh.h"
#include "Triangle.h"
#include "Matrix.h"
#include "Vector2.h"
#include <string>
#include "GameObject.h"
#include "ObjectManager.h"
#include "PhysXManager.h"
#include "Editor.h"
#include <chrono>
#include "LevelProperties.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Component.h"
#include "MeshComponent.h"

#include "iRender.h"

#include "Logger.h"
Spectral::Spectral() 
    : m_viewOffset(1, 1, 0)
    , m_deltaTime(0.0)
    , m_albedoBuffer(nullptr)
    , m_depthBuffer(nullptr)
    , m_render(true)
    , m_renderPhysX(false)
    , m_renderWireframe(false)
    , m_ambientLighting{0.1f,0.1f,0.1f,1.0f}
    , m_fogColor{1.0f,1.0f,1.0f,0.8f}
{
    LogMessage("Spectral initialized");

    m_directionLighting = Math::Vector3(0.1f, 1.0f, 0.8f);
    m_directionLighting = m_directionLighting.GetNormal();

}


Spectral::~Spectral()
{

}

void Spectral::RenderPhysX()
{
    const PxRenderBuffer& rb = PhysXManager::GetInstance()->GetScene()->getRenderBuffer();
    
    for (PxU32 i = 0; i < rb.getNbLines(); i++)
    {
        const PxDebugLine& line = rb.getLines()[i];

        Math::Vector3 p0(line.pos0);
        Math::Vector3 p1(line.pos1);

        Render::DrawLine(p0, p1);
    }
}

float Spectral::CalculateDeltaTime()
{
    static auto previousTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - previousTime).count();

    deltaTime = std::min(deltaTime, 0.1f);
    previousTime = currentTime;

    return deltaTime;
}


void Spectral::Update()
{
    m_deltaTime = CalculateDeltaTime();
#ifdef EDITOR
    static bool oldIsStarted = false;
    if (Editor::GetInstance()->IsStarted() == false)
    {
        if (oldIsStarted)
        {
            for (const auto& object : ObjectManager::GetInstance()->GetGameObjects())
            {
                for (const auto& component : object->GetComponents())
                {
                    component->Reset();
                }
            }
        }

        oldIsStarted = false;
        return;
    }
#endif
    if (oldIsStarted == false)
    {
        oldIsStarted = true;
        for (const auto& object : ObjectManager::GetInstance()->GetGameObjects())
        {
            for (const auto& component : object->GetComponents())
            {
                component->Start();
            }
        }
    }
    PhysXManager::GetInstance()->TickSimulation(m_deltaTime);
    for (const auto& object : ObjectManager::GetInstance()->GetGameObjects())
    {
        for (const auto& component : object->GetComponents())
        {
            component->Update(m_deltaTime);
        }
    }
}

void Spectral::SetAmbientLight(const ColorFLOAT& color)
{
    m_ambientLighting = color;
}

void Spectral::SetFogColor(const ColorFLOAT& color)
{
    m_fogColor = color;
}


void Spectral::Render()
{
    m_ligts.clear();

    for (const auto& object : ObjectManager::GetInstance()->GetGameObjects())
    {
        for (const auto& component : object->GetComponents())
        {
            component->Render();
        }
    }

    if (m_renderPhysX)
    {
        RenderPhysX();
    }
}