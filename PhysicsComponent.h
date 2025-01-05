#pragma once
#include "pch.h"
#include "Component.h"
#include "PhysXManager.h"

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
	void Reset() override;
	void Update(float deltaTime) override;
	void SaveComponent(rapidjson::Value& object, rapidjson::Document::AllocatorType& allocator) override;
	void LoadComponent(const rapidjson::Value& object) override;

	physx::PxRigidActor* GetActor() { return m_actor; }
	void SetActor(physx::PxRigidActor* actor) { m_actor = actor; }

	PhysXManager::PhysicsShape GetShapeType() const { return m_selectedShapeType; }



#ifdef EDITOR
	void ComponentEditor() override;
#endif // EDITOR

private:
	void UpdatePhysicsPosition();
	void UpdateShapeScale();

	void AddNewShape(PhysXManager::PhysicsShape shapeType);

	PhysXManager::PhysicsShape m_selectedShapeType;

	Math::Matrix m_initialMatrix;

	physx::PxRigidActor* m_actor;
	Math::Vector3 m_oldOwnerScale;


};

