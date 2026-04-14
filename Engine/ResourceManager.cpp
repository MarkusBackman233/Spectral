#include "ResourceManager.h"

ResourceManager::ResourceManager()
{
    m_resources.try_emplace(std::type_index(typeid(Texture)));
    m_resources.try_emplace(std::type_index(typeid(AudioSource)));
    m_resources.try_emplace(std::type_index(typeid(Model)));
    m_resources.try_emplace(std::type_index(typeid(DefaultMaterial)));
    m_resources.try_emplace(std::type_index(typeid(Script)));
    m_resources.try_emplace(std::type_index(typeid(Prefab)));
}

void ResourceManager::SaveResources()
{
    for (auto& resourceType : m_resources)
    {
        for (auto& resource : resourceType.second.StoredResources)
        {
            if (resource.second->IsUnsaved())
            {
                resource.second->Save();
                resource.second->SetSaved();
            }
        }
    }
}
