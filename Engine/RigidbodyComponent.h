#pragma once
#include "Component.h"
#include "PhysXManager.h"

class RigidbodyComponent :
    public Component
{
public:
    RigidbodyComponent(GameObject* owner);
    RigidbodyComponent(GameObject* owner, RigidbodyComponent* rigidbodyComponent);
	Component::Type GetComponentType() override { return Component::Type::Rigidbody; };
	~RigidbodyComponent();
	void Start() override;
	void Reset() override;
	void Update(float deltaTime) override;
	Json::Object SaveComponent() override;
	void LoadComponent(const rapidjson::Value& object) override;
#ifdef EDITOR
	void ComponentEditor() override;
#endif // EDITOR
	void UpdatePhysicsPosition();


	physx::PxRigidActor* GetActor() { return m_actor; }
	void SetActor(physx::PxRigidActor* actor) { m_actor = actor; }

	PhysXManager::PhysicsType GetPhysicsType() const { return m_physicsType; }
	void SetPhysicsType(PhysXManager::PhysicsType physicsType);

private:

	void CollectShapes(RigidbodyComponent* collector, GameObject* gameObject);

	Math::Vector3 m_interpolatePositions[2];

	Math::Matrix m_initialMatrix;
	physx::PxRigidActor* m_actor;
	PhysXManager::PhysicsType m_physicsType;

	std::vector<PxJoint*> m_runtimeJoints;

};

