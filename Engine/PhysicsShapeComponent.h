#pragma once
#include "Component.h"
#include "PhysXManager.h"

class PhysicsShapeComponent :
	public Component
{
public:
	PhysicsShapeComponent(GameObject* owner);
	PhysicsShapeComponent(GameObject* owner, PhysicsShapeComponent* physicsShapeComponent);
	Component::Type GetComponentType() override { return Component::Type::PhysicsShape; };
	~PhysicsShapeComponent();
	void Start() override;
	//void Reset() override;
	//void Update(float deltaTime) override;

	Json::Object SaveComponent() override;
	void LoadComponent(const rapidjson::Value& object) override;
#ifdef EDITOR
	void ComponentEditor() override;
#endif // EDITOR

	PxShape* GetShape() { return m_shape; }
	PhysXManager::PhysicsShape GetShapeType() const { return m_selectedShapeType; }

	GameObject* GetClosestActorInHierarchy(GameObject* gameObject);

private:

	void SetShape(PhysXManager::PhysicsShape shapeType);
	PhysXManager::PhysicsShape m_selectedShapeType;

	PxShape* m_shape;
};

