#pragma once
#include <memory>
#include "Component.h"

class Mesh;
class Material;
struct aiMesh;

class MeshComponent : public Component, public std::enable_shared_from_this<MeshComponent>
{
public:
	MeshComponent(GameObject* owner);
	MeshComponent(GameObject* owner, MeshComponent* meshComponent);
	MeshComponent(GameObject* owner, std::shared_ptr<Mesh> mesh);

	Component::Type GetComponentType() override { return Component::Type::Mesh; };

	void Render() override;
	void Update(float deltaTime) override;
	Json::Object SaveComponent() override;
	void LoadComponent(const rapidjson::Value& object) override;

#ifdef EDITOR
	void ComponentEditor() override;
#endif // EDITOR

	std::shared_ptr<Mesh> GetMesh() { return m_mesh; }
	void SetMesh(std::shared_ptr<Mesh> mesh);

	void SetMaterial(std::shared_ptr<Material> material);
	std::shared_ptr<Material> GetMaterial() { return m_material; }

private:

	std::shared_ptr<Mesh> m_mesh;
	std::shared_ptr<Material> m_material;
};

