#pragma once
#include "Component.h"
#include "PhysXManager.h"

class CharacterControllerComponent :
    public Component
{
public:
	CharacterControllerComponent(GameObject* owner);
	CharacterControllerComponent(GameObject* owner, CharacterControllerComponent* characterControllerComponent);
	Component::Type GetComponentType() override { return Component::Type::CharacterController; };
	~CharacterControllerComponent();

	void Start() override;
	void Reset() override;
	void Update(float deltaTime) override;
	Json::Object SaveComponent() override;
	void LoadComponent(const rapidjson::Value& object) override;
#ifdef EDITOR
	void ComponentEditor() override;
#endif // EDITOR

	bool IsOnGround() const;
	void Move(const Math::Vector3& displacement);

private:
	PxController* m_controller;
	Math::Vector3 m_moveDisplacement;

	bool m_isOnGround;
};

