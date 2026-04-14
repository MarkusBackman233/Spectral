#pragma once
#include <memory>
#include "Component.h"
#include "Matrix.h"
class Mesh;
class DefaultMaterial;
class Model;
struct SubMesh;

namespace physx
{
	class PxRigidActor;
}
class MeshComponent : public Component, public std::enable_shared_from_this<MeshComponent>
{
public:
	MeshComponent(GameObject* owner);
	MeshComponent(GameObject* owner, MeshComponent* meshComponent);
	MeshComponent(GameObject* owner, std::shared_ptr<Model> mesh);

	~MeshComponent();

	Component::Type GetComponentType() override { return Component::Type::Mesh; };

	void Start() override;
	void Reset() override;
	void Render() override;
	void Update(float deltaTime) override;
	Json::Object SaveComponent() override;
	void LoadComponent(const rapidjson::Value& object) override;

#ifdef EDITOR
	void ComponentEditor() override;
#endif // EDITOR

	std::shared_ptr<Model> GetMesh() { return m_mesh; }
	void SetMesh(std::shared_ptr<Model> mesh);


private:
	void RenderSubMesh(const SubMesh& subMesh);
	std::shared_ptr<Model> m_mesh;
	bool EditMaterial(SubMesh& subMesh);

	void SetPhysicsType(int physicsType);

	Math::Matrix m_initialMatrix;
	physx::PxRigidActor* m_actor = nullptr;
	float m_mass = 1.0f;
	int m_physicsType = 0;
};

