#include "JointComponent.h"
#include "src/IMGUI/imgui.h"
#include "GameObject.h"
#include "RigidbodyComponent.h"
#include "iRender.h"

std::map<JointComponent::JointType, std::string> JointComponent::JointTypeToString = {
	{JointComponent::JointType::Revolute, "Revolute" },
	{JointComponent::JointType::Spring, "Spring" },
	{JointComponent::JointType::Piston, "Piston" },
};

JointComponent::JointComponent(GameObject* owner) 
	: Component(owner)
	, m_jointType(JointType::Revolute)
{
}

JointComponent::JointComponent(GameObject* owner, JointComponent* JointComponent)
	: Component(owner)
	, m_jointType(JointComponent->GetJointType())
{
}

JointComponent::~JointComponent()
{

}

void JointComponent::Start()
{
	if (GetClosestActorInHierarchy(m_owner) == nullptr)
	{
		Logger::Error(std::string("joint object,") + m_owner->GetName() + " does not have a rigidbody component in its hierarchy!");
	}
}

void JointComponent::Render()
{
	Render::DrawLine(m_owner->GetWorldMatrix().GetPosition(), m_owner->GetWorldMatrix().GetPosition() + m_owner->GetWorldMatrix().GetRight());
}

Json::Object JointComponent::SaveComponent()
{
	return Json::Object{ {"Joint Type", JointTypeToString.at(m_jointType)} };
}

void JointComponent::LoadComponent(const rapidjson::Value& object)
{
	for (size_t i = 0; i < JointTypeToString.size(); i++)
	{
		if (strstr(object["Joint Type"].GetString(), JointTypeToString.at(static_cast<JointType>(i)).c_str()))
		{
			m_jointType = static_cast<JointType>(i);
			return;
		}
	}
	Assert(false, "Cound load joint type");
}
#ifdef EDITOR
void JointComponent::ComponentEditor()
{
	if (ImGui::BeginCombo("Type", JointTypeToString.find(m_jointType)->second.c_str()))
	{
		ImGui::Separator();
		for (const auto& [ShapeType, ShapeName] : JointTypeToString)
		{
			bool isSelected = ShapeType == m_jointType;
			if (ImGui::Selectable(ShapeName.c_str(), isSelected))
			{
				m_jointType = ShapeType;
			}
		}
		ImGui::EndCombo();
	}
}
#endif // DEBUG



PxRigidActor* JointComponent::GetClosestActorInHierarchy(GameObject* gameObject)
{
	if (auto rigidComp = gameObject->GetComponentOfType<RigidbodyComponent>())
	{
		return rigidComp->GetActor();
	}
	
	if (gameObject->GetParent())
	{
		return GetClosestActorInHierarchy(gameObject->GetParent());
	}

	return nullptr;
}
