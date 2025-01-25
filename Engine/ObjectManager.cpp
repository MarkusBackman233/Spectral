#include "ObjectManager.h"
#include <iostream>
#include "Logger.h"
#include "Component.h"
#include "CameraComponent.h"
#include "ScriptComponent.h"

ObjectManager::ObjectManager() : m_currentlyReserved(1000)
{
    Logger::Info("ObjectManager initialized");
    m_gameObjects.reserve(m_currentlyReserved);
}

void ObjectManager::Deinitialize()
{
    m_gameObjects.clear();
}

GameObject* ObjectManager::CreateObject(const std::string& name, bool destroyObjectOnReset /* = false*/)
{

    if (m_gameObjects.size() >= m_currentlyReserved)
    {
        m_currentlyReserved += 10000;
        m_gameObjects.reserve(m_currentlyReserved);
    }

	auto it = m_gameObjects.emplace(std::make_unique<GameObject>());
    GameObject* gameObject = it.first->get();
    gameObject->SetName(name);

    if (destroyObjectOnReset)
    {
        gameObject->DestroyOnReset();
    }

    return gameObject;
}

void ObjectManager::Destroy(GameObject* gameObject)
{
    Assert(gameObject != nullptr, "Cannot destroy null gameObject!");
    const std::vector<GameObject*> children = gameObject->GetChildren();
    for (GameObject* child : children)
    {
        Destroy(child);
    }

    gameObject->SetParent(nullptr);

    auto it = std::find_if(m_gameObjects.begin(), m_gameObjects.end(),
        [gameObject](const std::unique_ptr<GameObject>& p) { return p->GetId() == gameObject->GetId(); });
    if (it != m_gameObjects.end())
    {
        m_gameObjects.erase(it);
    }
}

void ObjectManager::Start()
{
    for (auto& object : m_gameObjects) // need to run script start before other component starts
    {
        for (const auto& component : object->GetComponents())
        {
            

            if (component->Is<ScriptComponent>())
            {
                component->Start();
            }
        }
    }
    for (auto& object : m_gameObjects)
    {
        object->Start();
        for (const auto& component : object->GetComponents())
        {
            if (component->Is<ScriptComponent>() == nullptr)
            {
                component->Start();
            }
        }
    }
}

void ObjectManager::Reset()
{
    std::vector<GameObject*> objectsToDestroy;


    for (auto& object : m_gameObjects)
    {
        if (object->ShouldDestroyOnReset())
        {
            objectsToDestroy.push_back(object.get());
        }

        object->Reset();
        for (const auto& component : object->GetComponents())
        {
            component->Reset();
        }
    }

    for (size_t i = 0; i < objectsToDestroy.size(); i++)
    {
        Destroy(objectsToDestroy[i]);
    }
}

void ObjectManager::Update(float deltaTime)
{
    for (const auto& object : m_gameObjects)
    {
        for (const auto& component : object->GetComponents())
        {
            component->Update(deltaTime);
        }
    }
}

void ObjectManager::Render()
{
    for (const auto& object : m_gameObjects)
    {
        for (const auto& component : object->GetComponents())
        {
            component->Render();
        }
    }
}

GameObject* ObjectManager::GetMainCameraGameObject()
{
    const auto& gameObjects = ObjectManager::GetInstance()->GetGameObjects();
    for (const auto& gameObject : gameObjects)
    {
        if (const auto& cameraComponent = gameObject->GetComponentOfType<CameraComponent>())
        {
            return gameObject.get();
        }
    }
    return nullptr;
}
