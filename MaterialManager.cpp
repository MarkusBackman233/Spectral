#include "MaterialManager.h"
#include "TextureManager.h"
#include "Material.h"
#include "IOManager.h"

MaterialManager::MaterialManager()
{
	m_materials["default"] = std::make_shared<Material>();
	m_materials["default"]->SetName("default");
}

void MaterialManager::AddMaterial(std::shared_ptr<Material> material)
{
	std::unique_lock lock(m_mutex); // Exclusive lock for writing
	m_materials[material->GetName()] = material;
}

std::shared_ptr<Material> MaterialManager::GetDefaultMaterial()
{
	return m_materials["default"];
}

std::shared_ptr<Material> MaterialManager::GetMaterial(const std::string& name)
{
	{
		std::shared_lock lock(m_mutex); // Shared lock for reading

		auto materialIt = m_materials.find(name);
		if (materialIt != m_materials.end())
		{
			return m_materials[name];
		}
	}

	std::shared_ptr<Material> material = std::make_shared<Material>();
	{
		std::unique_lock lock(m_mutex); // Exclusive lock for writing
		m_materials.try_emplace(name, material);
	}
	Logger::Info("Loading Material: " + name);
	if (IOManager::LoadSpectralMaterial(name, material))
	{
		return material;
	}

	material->SetTexture(0, TextureManager::GetInstance()->GetTexture("TemplateGrid_albedo.bmp"));
	material->SetTexture(1, TextureManager::GetInstance()->GetTexture("TemplateGrid_normal.bmp"));
	material->SetName(name);
	return material;
}