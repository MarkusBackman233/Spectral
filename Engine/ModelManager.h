#pragma once
#include "pch.h"
#include <shared_mutex>

class Mesh;

class ModelManager
{
public:
	static ModelManager* GetInstance() {
		static ModelManager instance;
		return &instance;
	}

	std::shared_ptr<Mesh> GetMesh(const std::string& filename);
	void AddMesh(const std::string& filename, std::shared_ptr<Mesh> mesh);

	std::unordered_map<std::string, std::shared_ptr<Mesh>> GetCachedMeshes() { return m_meshes; }
private:
	mutable std::shared_mutex m_mutex;
	std::unordered_map<std::string, std::shared_ptr<Mesh>> m_meshes;
};

