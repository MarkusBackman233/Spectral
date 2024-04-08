#include "PhysicsComponent.h"
#include "PhysXManager.h"
#include "GameObject.h"

PhysicsComponent::PhysicsComponent(GameObject* owner)
	: Component(owner)
	, m_actor(nullptr)
	, m_oldOwnerScale{}
{
	m_componentName = "PhysicsComponent";
	m_componentType = ComponentType_PhysicsComponent;

	auto newActor = PhysXManager::GetInstance()->CreateActor(PhysXManager::PhysicsType::DynamicActor, m_owner->GetMatrix());
	SetActor(newActor);
	auto defaultShape = PhysXManager::GetInstance()->CreateBoxShape(m_owner->GetMatrix().GetScale());
	GetActor()->attachShape(*defaultShape);
}

PhysicsComponent::PhysicsComponent(GameObject* owner, PhysicsComponent* physicsComponent)
	: Component(owner)
{
	m_componentName = "PhysicsComponent";
	m_componentType = ComponentType_PhysicsComponent;
	auto newActor = PhysXManager::GetInstance()->CreateActor(physicsComponent->GetActor());
	SetActor(newActor);

	PxU32 numShapes = physicsComponent->GetActor()->getNbShapes();
	PxShape** shapes = new PxShape * [numShapes];
	physicsComponent->GetActor()->getShapes(shapes, numShapes);

	for (PxU32 i = 0; i < numShapes; ++i) {
		PxTransform transform = shapes[i]->getLocalPose();
		PxGeometryHolder geometry = shapes[i]->getGeometry();

		PxShape* newShape = PhysXManager::GetInstance()->GetPhysics()->createShape(geometry.any(), *PhysXManager::GetInstance()->GetPhysics()->createMaterial(0.5f, 0.5f, 0.1f), true);
		newShape->setLocalPose(transform);

		newActor->attachShape(*newShape);
	}

	delete[] shapes;
}

PhysicsComponent::~PhysicsComponent()
{
	m_actor->release();
}



void PhysicsComponent::Start()
{
	auto actor = GetActor();

	actor->setGlobalPose(PhysXManager::GetInstance()->MatrixToPxTransform(m_owner->GetMatrix()));
	if (actor->is<PxRigidDynamic>()) // This does not work
	{
		auto rigidbodyFlags = actor->is<PxRigidDynamic>()->getRigidBodyFlags();
		bool isKinematic = rigidbodyFlags & PxRigidBodyFlag::eKINEMATIC;
		if (isKinematic == false)
		{
			actor->is<PxRigidDynamic>()->clearForce(PxForceMode::eFORCE);
			actor->is<PxRigidDynamic>()->clearForce(PxForceMode::eACCELERATION);
			actor->is<PxRigidDynamic>()->clearForce(PxForceMode::eIMPULSE);
			actor->is<PxRigidDynamic>()->clearForce(PxForceMode::eVELOCITY_CHANGE);
			actor->is<PxRigidDynamic>()->clearTorque(PxForceMode::eFORCE);
			actor->is<PxRigidDynamic>()->clearTorque(PxForceMode::eACCELERATION);
			actor->is<PxRigidDynamic>()->clearTorque(PxForceMode::eIMPULSE);
			actor->is<PxRigidDynamic>()->clearTorque(PxForceMode::eVELOCITY_CHANGE);
		}
	}
}

void PhysicsComponent::Update(float deltaTime)
{
	if (m_actor && m_actor->is<PxRigidDynamic>())
	{
		UpdatePhysicsPosition(); // Check if simulation is running
	}
}

#ifdef EDITOR
void PhysicsComponent::ComponentEditor()
{
	auto rigidDynamic = GetActor()->is<PxRigidDynamic>();

	if (rigidDynamic)
	{
		float mass = rigidDynamic->getMass();
		ImGui::PushItemWidth(104);
		if (ImGui::DragFloat("Mass", &mass))
		{
			rigidDynamic->setMass(mass);
		}
		ImGui::PopItemWidth();
		auto rigidbodyFlags = rigidDynamic->getRigidBodyFlags();
		bool isKinematic = rigidbodyFlags & PxRigidBodyFlag::eKINEMATIC;

		if (ImGui::Checkbox("Kinematic", &isKinematic))
		{
			rigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, isKinematic);
		}
	}

	const char* items[] = { "Box", "Sphere", "Convex", "Triangle" };
	static int item_current = 0;
	int oldSelectedItem = item_current;
	if (ImGui::Combo("Collider Type", &item_current, items, IM_ARRAYSIZE(items)))
	{
		if (oldSelectedItem != item_current)
		{
			PhysXManager::GetInstance()->DetachShapesFromActor(GetActor());
			switch (item_current)
			{
			case 0:
			{
				auto boxShape = PhysXManager::GetInstance()->CreateBoxShape(m_owner->GetMatrix().GetScale());
				GetActor()->attachShape(*boxShape);
				break;
			}
			case 1:
			{
				auto sphereShape = PhysXManager::GetInstance()->CreateSphereShape(m_owner->GetMatrix().GetScale().x);
				GetActor()->attachShape(*sphereShape);
				break;
			}
			}
		}
	}
}
#endif

void PhysicsComponent::UpdatePhysicsPosition()
{
	Math::Vector3 scale = m_owner->GetMatrix().GetScale();

	if (m_oldOwnerScale != scale)
	{
		UpdateShapeScale();
		m_oldOwnerScale = scale;
	}

	auto&ownerMatrix = m_owner->GetMatrix();
	ownerMatrix.SetRight(m_actor->getGlobalPose().q.getBasisVector0() * scale.x);
	ownerMatrix.SetFront(m_actor->getGlobalPose().q.getBasisVector2() * scale.z);
	ownerMatrix.SetUp(m_actor->getGlobalPose().q.getBasisVector1() * scale.y);
	ownerMatrix.SetPosition(m_actor->getGlobalPose().p);

	for (auto& child : m_owner->GetChildren())
	{
		child->GetMatrix() = child->GetLocalMatrix() * m_owner->GetMatrix();
	}
}

void PhysicsComponent::UpdateShapeScale()
{
	PxU32 numShapes = m_actor->getNbShapes();
	PxShape** shapes = new PxShape * [numShapes];
	m_actor->getShapes(shapes, numShapes);
	for (PxU32 i = 0; i < numShapes; i++)
	{
		switch (shapes[i]->getGeometryType())
		{
			case PxGeometryType::eBOX:
			{
				auto boxShape = PhysXManager::GetInstance()->CreateBoxShape(m_owner->GetMatrix().GetScale());
				GetActor()->attachShape(*boxShape);
				break;
			}
			case PxGeometryType::eSPHERE:
			{
				auto boxShape = PhysXManager::GetInstance()->CreateSphereShape(m_owner->GetMatrix().GetScale().Length());
				GetActor()->attachShape(*boxShape);
				break;
			}
		}

		m_actor->detachShape(*shapes[i]);
	}
	delete[] shapes;
}
