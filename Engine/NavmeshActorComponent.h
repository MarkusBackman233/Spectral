#pragma once
#include "Component.h"
#include "PhysXManager.h"
#include "DetourCrowd.h"
class NavmeshActorComponent :
    public Component
{
public:
	NavmeshActorComponent(GameObject* owner);
	NavmeshActorComponent(GameObject* owner, NavmeshActorComponent* characterControllerComponent);
	Component::Type GetComponentType() override { return Component::Type::NavmeshActor; };
	~NavmeshActorComponent();

	void Start() override;
	void Reset() override;
	void Update(float deltaTime) override;
	Json::Object SaveComponent() override;
	void LoadComponent(const rapidjson::Value& object) override;
#ifdef EDITOR
	void ComponentEditor() override;
#endif // EDITOR

	void SetTarget(const Math::Vector3& target);
	bool HasTarget() const;

	dtCrowdAgentParams GetAgentParams() const;

private:
	dtCrowdAgentParams m_agentParams;
	void CreateAgent();
	float m_yrot{};

	Math::Vector3 m_target;
	bool m_hasTarget{};

	int m_agentId = -1;
};

