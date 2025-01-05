#pragma once
#include "Component.h"
#include "PhysXManager.h"

class JointComponent :
	public Component
{
public:

	enum class JointType : uint8_t
	{
		Revolute,
		Piston,
		Spring,

		Num
	};

	JointComponent(GameObject* owner);
	JointComponent(GameObject* owner, JointComponent* JointComponent);
	Component::Type GetComponentType() override { return Component::Type::Joint; };
	~JointComponent();
	void Start() override;
	void Render() override;
	//void Update(float deltaTime) override;

	void SaveComponent(rapidjson::Value& object, rapidjson::Document::AllocatorType& allocator) override;
	void LoadComponent(const rapidjson::Value& object) override;
#ifdef EDITOR
	void ComponentEditor() override;
#endif // EDITOR
	JointType GetJointType() const { return m_jointType; }

	static std::map<JointComponent::JointType, std::string> JointTypeToString;

private:
	PxRigidActor* GetClosestActorInHierarchy(GameObject* gameObject);

	JointType m_jointType;
};

