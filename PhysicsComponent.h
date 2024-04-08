#pragma once
#include "pch.h"
#include "Component.h"
#include "Mesh.h"
#include <string>

namespace physx
{
	class PxRigidActor;
}

class PhysicsComponent : public Component
{
public:
	PhysicsComponent(GameObject* owner);
	PhysicsComponent(GameObject* owner, PhysicsComponent* physicsComponent);
	~PhysicsComponent();
	void Start() override;
	void Update(float deltaTime) override;

	physx::PxRigidActor* GetActor() { return m_actor; }
	void SetActor(physx::PxRigidActor* actor) { m_actor = actor; }

#ifdef EDITOR
	void ComponentEditor() override;
#endif // EDITOR

private:
	void UpdatePhysicsPosition();
	void UpdateShapeScale();

	physx::PxRigidActor* m_actor;
	Math::Vector3 m_oldOwnerScale;
};

