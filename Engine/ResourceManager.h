#pragma once
#include <pch.h>
#include "LoadableResource.h"
#include <typeindex>
#include <shared_mutex>
#include "SpectralAssert.h"
#include "Texture.h"
#include "AudioSource.h"
#include "Mesh.h"
#include "Material.h"
#include "Script.h"
#include "IOManager.h"
#include "Prefab.h"
class ResourceManager
{
public:
	static ResourceManager* GetInstance() {
		static ResourceManager instance;
		return &instance;
	}

    ResourceManager();

	template<typename T>
	std::shared_ptr<T> GetResource(const std::filesystem::path& file)
	{
        static_assert(std::is_base_of<Resource, T>::value, "T must inherit from Resource");

        ResourceData& resourceData = m_resources[std::type_index(typeid(T))];

        std::shared_ptr<T> resource(nullptr);
        std::string filename(file.filename().string().c_str());

        {
            std::shared_lock lock(resourceData.Mutex); // Shared lock for reading
            
            auto it = resourceData.StoredResources.find(filename);
            if (it != resourceData.StoredResources.end())
            {
                return std::dynamic_pointer_cast<T>(it->second);
            }
        }
        {
            resource = std::make_shared<T>();
            std::unique_lock lock(resourceData.Mutex);
            resourceData.StoredResources.try_emplace(filename, resource);
        }



        std::filesystem::path foundPath;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(IOManager::ProjectDirectory))
        {
            if (entry.is_regular_file() && entry.path().filename() == filename)
            {
                foundPath = entry.path();
                break;
            }
        }

        if (foundPath.empty())
        {
            Logger::Error("File not found: " + filename);
            std::unique_lock lock(resourceData.Mutex); // Exclusive lock for writing
            resourceData.StoredResources.erase(filename);
            return nullptr;
        }

        if (!resource->Load(foundPath))
        {
            std::unique_lock lock(resourceData.Mutex); // Exclusive lock for writing
            resourceData.StoredResources.erase(filename);
            return nullptr;
        }
        resource->m_filename = filename;
        return resource;
	}


    template<typename T>
    std::vector<std::shared_ptr<T>> GetResources()
    {
        static_assert(std::is_base_of<Resource, T>::value, "T must inherit from Resource");

        ResourceData& resourceData = m_resources[std::type_index(typeid(T))];

        std::shared_lock lock(resourceData.Mutex); // Shared lock for reading

        std::vector<std::shared_ptr<T>> typedResources;
        for (const auto& [name, resource] : resourceData.StoredResources)
        {
            typedResources.push_back(std::dynamic_pointer_cast<T>(resource));
        }

        return typedResources;
    }

    template<typename T>
    void AddResource(std::shared_ptr<Resource> resource)
    {
        static_assert(std::is_base_of<Resource, T>::value, "T must inherit from Resource");
        Assert(!resource->GetFilename().empty(),"Filename cant be empty");

        ResourceData& resourceData = m_resources[std::type_index(typeid(T))];

        std::unique_lock lock(resourceData.Mutex); // Shared lock for writing
        resourceData.StoredResources.try_emplace(resource->m_filename, resource);
    }

private:

	struct ResourceData
	{
		std::shared_mutex Mutex;
		std::unordered_map<std::string, std::shared_ptr<Resource>> StoredResources;
	};

	std::unordered_map<std::type_index, ResourceData> m_resources;
};

