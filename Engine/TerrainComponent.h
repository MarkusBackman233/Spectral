#pragma once
#include "Component.h"
#include "pch.h"
class Mesh;
class Material;

class TerrainComponent : public Component
{
public:
	TerrainComponent(GameObject* owner);
	Component::Type GetComponentType() override { return Component::Type::Terrain; };
	void Render() override;
	void Update(float deltaTime) override;

	Json::Object SaveComponent() override;
	void LoadComponent(const rapidjson::Value& object) override;

	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> GetMaterial();

#ifdef EDITOR
	 void ComponentEditor() override;
	 void DisplayComponentIcon() override;
#endif // EDITOR

private:
	float m_brushSize;
	void CreatePlaneMesh();
	std::shared_ptr<Mesh> m_mesh;
	std::shared_ptr<Material> m_material;

};

