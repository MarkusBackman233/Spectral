#include "MaterialManager.h"
#include "TextureManager.h"
#include "Material.h"
#include "LineMaterial.h"
#include <mutex>
#include "IOManager.h"

MaterialManager::MaterialManager()
{
	m_materials["default"] = std::make_shared<Material>();
	m_materials["default"]->SetTexture(0, TextureManager::GetInstance()->GetTexture("TemplateGrid_albedo.bmp"));
	m_materials["default"]->SetTexture(1, TextureManager::GetInstance()->GetTexture("TemplateGrid_normal.bmp"));
	m_materials["default"]->SetName("default");


	m_materials["line"] = std::make_shared<LineMaterial>();
	m_materials["line"]->SetName("line");
}

void MaterialManager::AddMaterial(std::shared_ptr<Material> material)
{
	m_materials[material->GetName()] = material;
}

std::shared_ptr<Material> MaterialManager::GetDefaultMaterial()
{
	return m_materials["default"];
}

std::shared_ptr<Material> MaterialManager::GetLineMaterial()
{
	return m_materials["line"];
}


std::shared_ptr<Material> MaterialManager::GetMaterial(const std::string& name)
{
	static std::mutex mutex;
	mutex.lock();
	auto materialIt = m_materials.find(name);
	if (materialIt != m_materials.end())
	{
		mutex.unlock();
		return m_materials[name];
	}
	m_materials[name] = std::make_shared<Material>();
	mutex.unlock();

	if (IOManager::LoadSpectralMaterial(name, m_materials[name]))
	{
		return m_materials[name];
	}

	m_materials[name]->SetTexture(0, TextureManager::GetInstance()->GetTexture("TemplateGrid_albedo.bmp"));
	m_materials[name]->SetTexture(1, TextureManager::GetInstance()->GetTexture("TemplateGrid_normal.bmp"));
	m_materials[name]->SetName(name);
	return m_materials[name];
}