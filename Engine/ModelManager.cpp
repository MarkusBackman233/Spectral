#include "ModelManager.h"
#include "Logger.h"
#include <shared_mutex>


std::shared_ptr<Mesh> ModelManager::GetMesh(const std::string& filename)
{
	std::shared_lock lock(m_mutex); // Shared lock for reading

	auto mesh = m_meshes.find(filename);
	if(mesh != m_meshes.end())
	{
		return mesh->second;
	}
	return nullptr;
}

void ModelManager::AddMesh(const std::string& filename, std::shared_ptr<Mesh> mesh)
{
	std::unique_lock lock(m_mutex); // Exclusive lock for writing

	m_meshes.try_emplace(filename, mesh);
}
