#include "PhysicsShapeComponent.h"
#include "RigidbodyComponent.h"
#include "src/IMGUI/imgui.h"

#include "MeshComponent.h"
#include "TerrainComponent.h"
#include "GameObject.h"
#include "Logger.h"

PhysicsShapeComponent::PhysicsShapeComponent(GameObject* owner) 
	: Component(owner)
	, m_shape(nullptr)
{
	SetShape(PhysXManager::PhysicsShape::Box);
}

PhysicsShapeComponent::PhysicsShapeComponent(GameObject* owner, PhysicsShapeComponent* physicsShapeComponent)
	: Component(owner)
	, m_shape(nullptr)
{
	SetShape(physicsShapeComponent->GetShapeType());
}

PhysicsShapeComponent::~PhysicsShapeComponent()
{
	if (m_shape)
	{
		m_shape->release();
		m_shape = nullptr;
	}
}

void PhysicsShapeComponent::Start()
{
	if (GetClosestActorInHierarchy(m_owner) == nullptr)
	{
		Logger::Error(std::string("Physics shape object,") + m_owner->GetName() + " does not have a rigidbody component in its hierarchy!");
	}
}

Json::Object PhysicsShapeComponent::SaveComponent()
{
	Json::Object object;

	object.emplace("Shape Type", (int)GetShapeType());

	return std::move(object);
}

void PhysicsShapeComponent::LoadComponent(const rapidjson::Value& object)
{
	SetShape(static_cast<PhysXManager::PhysicsShape>(object["Shape Type"].GetInt()));
}
#ifdef EDITOR
void PhysicsShapeComponent::ComponentEditor()
{
	if (ImGui::BeginCombo("Shape", PhysXManager::PhysicsShapeToString.find(m_selectedShapeType)->second.c_str()))
	{
		ImGui::Separator();
		for (const auto& [ShapeType, ShapeName] : PhysXManager::PhysicsShapeToString)
		{
			bool isSelected = ShapeType == m_selectedShapeType;
			if (ImGui::Selectable(ShapeName.c_str(), isSelected))
			{
				SetShape(ShapeType);
			}
		}
		ImGui::EndCombo();
	}
}
#endif // DEBUG

void PhysicsShapeComponent::SetShape(PhysXManager::PhysicsShape shapeType)
{
	m_selectedShapeType = shapeType;
}

PxRigidActor* PhysicsShapeComponent::GetClosestActorInHierarchy(GameObject* gameObject)
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
