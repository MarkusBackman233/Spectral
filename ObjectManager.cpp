#include "ObjectManager.h"
#include <iostream>
#include "Logger.h"

ObjectManager::ObjectManager()
{
    LogMessage("ObjectManager initialized");
}

GameObject* ObjectManager::CreateObject(const std::string& name)
{
    GameObject* newGameObject = new GameObject();
    newGameObject->SetName(name);
	m_gameObjects.push_back(newGameObject);
    return newGameObject;
}

void ObjectManager::Destroy(GameObject* gameObject)
{
    auto children = gameObject->GetChildren();
    for (auto& child : children)
    {
        Destroy(child);
    }
    if (gameObject->GetParent())
    {
        gameObject->GetParent()->RemoveChild(gameObject);
    }

    auto iter = std::find_if(m_gameObjects.begin(), m_gameObjects.end(),
        [&](GameObject* p) { return p == gameObject; });
    
    if (iter != m_gameObjects.end())
    {
        m_gameObjects.erase(iter);
    }
    delete gameObject;
}