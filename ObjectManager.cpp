#include "ObjectManager.h"
#include <iostream>
#include "Logger.h"
#include "Component.h"
#include "CameraComponent.h"
#include "ScriptComponent.h"

ObjectManager::ObjectManager()
{
    Logger::Info("ObjectManager initialized");
    m_gameObjects.reserve(10000);
}

void ObjectManager::Deinitialize()
{
    m_gameObjects.clear();
}

GameObject* ObjectManager::CreateObject(const std::string& name, bool destroyObjectOnReset /* = false*/)
{
	auto& gameObject = m_gameObjects.emplace_back(std::make_unique<GameObject>());
    gameObject->SetName(name);

    if (destroyObjectOnReset)
    {
        gameObject->DestroyOnReset();
    }

    return gameObject.get();
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

    auto iter = std::find_if(m_gameObjects.begin(), m_gameObjects.end(),
        [gameObject](std::unique_ptr<GameObject>& p) { return p->GetId() == gameObject->GetId(); });
    
    if (iter != m_gameObjects.end())
    {
        m_gameObjects.erase(iter);
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
