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
    for (auto& component : gameObject->GetComponents())
    {
        ObjectManager::GetInstance()->UnregisterComponent(component);
    }
    auto it = std::find_if(m_gameObjects.begin(), m_gameObjects.end(),
        [gameObject](const std::unique_ptr<GameObject>& p) { return p->GetId() == gameObject->GetId(); });
    if (it != m_gameObjects.end())
    {
        m_gameObjects.erase(it);
    }
}

void ObjectManager::RegisterComponent(std::shared_ptr<Component> component)
{
    std::shared_lock lock(m_getObjectsMutex); // Shared lock for reading
    auto type = component->GetComponentType();

    if (m_components.find(component->GetComponentType()) == m_components.end())
    {
        m_components[type] = std::vector<std::weak_ptr<Component>>();
    }

    m_components[type].push_back(component);
}

void ObjectManager::UnregisterComponent(std::shared_ptr<Component> component)
{
    auto type = component->GetComponentType();
    std::unique_lock lock(m_getObjectsMutex); // Exclusive lock for writing
    auto it = m_components.find(type);
    if (it != m_components.end()) {
        auto& componentList = it->second;

        componentList.erase(
            std::remove_if(componentList.begin(), componentList.end(),
                [&component](const std::weak_ptr<Component>& wptr) {
            auto sptr = wptr.lock();
            return sptr == nullptr || sptr == component;
        }),
            componentList.end()
        );
    }
}

std::vector<std::weak_ptr<Component>> ObjectManager::GetComponentsOfType(Component::Type type)
{
    std::unique_lock lock(m_getObjectsMutex); // Exclusive lock for writing

    auto it = m_components.find(type);
    if (it != m_components.end())
    {
        return it->second;
    }
    return std::vector<std::weak_ptr<Component>>();
}

void ObjectManager::Start()
{
    for (auto& object : m_gameObjects) 
    {
        object->Start();
    }

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
    auto cameras = ObjectManager::GetInstance()->GetComponentsOfType(Component::Type::Camera);
    for (auto& component : cameras)
    {
        return component.lock()->GetOwner();
    }
    return nullptr;

}
