#pragma once
#include "Component.h"
#include "pch.h"

class Mesh;

class TerrainComponent : public Component
{
public:
	TerrainComponent(GameObject* owner);
	
	void Render() override;
	void Update(float deltaTime) override;

#ifdef EDITOR
	 void ComponentEditor() override;
#endif // EDITOR

private:
	float m_brushSize;

	void CreatePlaneMesh();

	std::shared_ptr<Mesh> m_mesh;
};

