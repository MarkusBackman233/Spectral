#pragma once
#include "Component.h"
#include "pch.h"

class Mesh;

class TerrainComponent : public Component
{
public:
	TerrainComponent(GameObject* owner);
	Component::Type GetComponentType() override { return Component::Type::Terrain; };
	void Render() override;
	void Update(float deltaTime) override;

	void SaveComponent(rapidjson::Value& object, rapidjson::Document::AllocatorType& allocator) override;
	void LoadComponent(const rapidjson::Value& object) override;

	std::shared_ptr<Mesh> GetMesh();

#ifdef EDITOR
	 void ComponentEditor() override;
	 void DisplayComponentIcon() override;
#endif // EDITOR

private:
	float m_brushSize;
	void CreatePlaneMesh();
	std::shared_ptr<Mesh> m_mesh;
};

