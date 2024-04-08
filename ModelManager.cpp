#include "ModelManager.h"
#include "IOManager.h"
#include "Logger.h"
#include <mutex>
ModelManager::ModelManager()
{
	LogMessage("ModelManager initialized");
}

std::shared_ptr<Mesh> ModelManager::GetMesh(const std::string& filename)
{
	static std::mutex mutex;
	mutex.lock();
	auto mesh = m_meshes.find(filename);

	if(mesh != m_meshes.end())
	{
		mutex.unlock();
		return m_meshes[filename];
	}
	mutex.unlock();
	return nullptr;
}

void ModelManager::AddMesh(const std::string& filename, std::shared_ptr<Mesh> mesh)
{
	static std::mutex mutex;
	mutex.lock();
	m_meshes[filename] = mesh;
	mutex.unlock();
}

std::string ModelManager::GetMeshName(std::shared_ptr<Mesh> Mesh)
{
	for (const auto& [filename, cachedMesh] : m_meshes)
	{
		if (cachedMesh == Mesh)
		{
			return filename;
		}
	}

	return "";
}
