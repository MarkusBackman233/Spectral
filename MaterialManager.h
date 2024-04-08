#pragma once
#include "pch.h"
class Material;

class MaterialManager
{
public:

	static MaterialManager* GetInstance() {
		static MaterialManager instance;
		return &instance;
	}
	MaterialManager();

	void AddMaterial(std::shared_ptr<Material> material);
	std::shared_ptr<Material> GetDefaultMaterial();
	std::shared_ptr<Material> GetLineMaterial();

	std::shared_ptr<Material> GetMaterial(const std::string& name);
	std::unordered_map<std::string, std::shared_ptr<Material>>& GetMaterials() { return m_materials; }
	
private:

	std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;
};

