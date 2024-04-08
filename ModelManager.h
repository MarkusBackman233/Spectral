#pragma once
#include "pch.h"
#include <unordered_map>
#include <iostream>
#include "Mesh.h"
class shared_ptr;

class ModelManager
{
public:
	ModelManager();

	static ModelManager* GetInstance() {
		static ModelManager instance;
		return &instance;
	}

	std::shared_ptr<Mesh> GetMesh(const std::string& filename);

	void AddMesh(const std::string& filename, std::shared_ptr<Mesh> mesh);

	std::unordered_map<std::string, std::shared_ptr<Mesh>>& GetCachedMeshes() { return m_meshes; }
	std::string GetMeshName(std::shared_ptr<Mesh> Mesh);
private:
	
	std::unordered_map<std::string, std::shared_ptr<Mesh>> m_meshes;
};

