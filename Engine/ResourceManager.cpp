#include "ResourceManager.h"

ResourceManager::ResourceManager()
{

    m_resources.try_emplace(std::type_index(typeid(Texture)));
    m_resources.try_emplace(std::type_index(typeid(AudioSource)));
    m_resources.try_emplace(std::type_index(typeid(Mesh)));
    m_resources.try_emplace(std::type_index(typeid(Material)));
}
