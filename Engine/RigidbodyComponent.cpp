#include "RigidbodyComponent.h"

#include "PhysicsShapeComponent.h"
#include "PhysXManager.h"
#include "src/IMGUI/imgui.h"
#include "GameObject.h"
#include "MeshComponent.h"
#include "TerrainComponent.h"
#include "JointComponent.h"
#include "Mesh.h"
RigidbodyComponent::RigidbodyComponent(GameObject* owner)
	: Component(owner)
	, m_actor(nullptr)
	, m_physicsType(PhysXManager::PhysicsType::StaticActor)
{

	auto newActor = PhysXManager::GetInstance()->CreateActor(PhysXManager::PhysicsType::StaticActor, m_owner->GetWorldMatrix());
	SetActor(newActor);
}

RigidbodyComponent::RigidbodyComponent(GameObject* owner, RigidbodyComponent* rigidbodyComponent)
	: Component(owner)
	, m_actor(nullptr)
{
	m_physicsType = rigidbodyComponent->GetPhysicsType();
	SetActor(PhysXManager::GetInstance()->CreateActor(rigidbodyComponent->GetActor()));
}

RigidbodyComponent::~RigidbodyComponent()
{
	if (m_actor)
	{
		m_actor->release();
		m_actor = nullptr;
	}
}

void RigidbodyComponent::Start()
{

	auto actor = GetActor();
	PhysXManager::GetInstance()->DetachShapesFromActor(actor);

	CollectShapes(this, m_owner);

	actor->setGlobalPose(PhysXManager::MatrixToPxTransform(m_owner->GetWorldMatrix()));

	if (auto rigidDynamic = actor->is<PxRigidDynamic>())
	{
		m_initialMatrix = m_owner->GetWorldMatrix();
		auto rigidbodyFlags = rigidDynamic->getRigidBodyFlags();
		bool isKinematic = rigidbodyFlags & PxRigidBodyFlag::eKINEMATIC;
		if (isKinematic == false)
		{
			rigidDynamic->setLinearVelocity(PxVec3(0.0f, 0.0f, 0.0f));
			rigidDynamic->setAngularVelocity(PxVec3(0.0f, 0.0f, 0.0f));
			rigidDynamic->clearForce(PxForceMode::eFORCE);
			rigidDynamic->clearForce(PxForceMode::eACCELERATION);
			rigidDynamic->clearForce(PxForceMode::eIMPULSE);
			rigidDynamic->clearForce(PxForceMode::eVELOCITY_CHANGE);
			rigidDynamic->clearTorque(PxForceMode::eFORCE);
			rigidDynamic->clearTorque(PxForceMode::eACCELERATION);
			rigidDynamic->clearTorque(PxForceMode::eIMPULSE);
			rigidDynamic->clearTorque(PxForceMode::eVELOCITY_CHANGE);
		}
	}
}

void RigidbodyComponent::Reset()
{
	if (m_physicsType == PhysXManager::PhysicsType::DynamicActor)
	{
		m_owner->SetWorldMatrix(m_initialMatrix);
	}

	for (size_t i = 0; i < m_runtimeJoints.size(); i++)
	{
		m_runtimeJoints[i]->release();
	}
	m_runtimeJoints.clear();
}

void RigidbodyComponent::Update(float deltaTime)
{
	if (m_actor && m_actor->is<PxRigidDynamic>())
	{
		UpdatePhysicsPosition();
	}
}

void RigidbodyComponent::SaveComponent(rapidjson::Value& object, rapidjson::Document::AllocatorType& allocator)
{
	object.AddMember("Physics Type", GetPhysicsType(), allocator);
	if (auto dynamicActor = m_actor->is<PxRigidDynamic>())
	{
		object.AddMember("Mass", dynamicActor->getMass(), allocator);
	}
}

void RigidbodyComponent::LoadComponent(const rapidjson::Value& object)
{
	SetPhysicsType(static_cast<PhysXManager::PhysicsType>(object["Physics Type"].GetInt()));
	if (auto dynamicActor = m_actor->is<PxRigidDynamic>())
	{
		if (object.HasMember("Mass"))
		{
			dynamicActor->setMass(object["Mass"].GetFloat());
		}
	}
}

#ifdef EDITOR
void RigidbodyComponent::ComponentEditor()
{
	static  std::unordered_map<PhysXManager::PhysicsType, std::string> rigidbodyTypes = 
	   {{PhysXManager::PhysicsType::StaticActor, "Static"},
		{PhysXManager::PhysicsType::DynamicActor, "Dynamic"},
		{PhysXManager::PhysicsType::VehicleActor, "Vehicle"}};

	if (ImGui::BeginCombo("Rigidbody Type", rigidbodyTypes.find(m_physicsType)->second.c_str()))
	{
		ImGui::Separator();
		for (const auto& [rigidbodyType, name] : rigidbodyTypes)
		{
			bool isSelected = rigidbodyType == m_physicsType;
			if (ImGui::Selectable(name.c_str(), isSelected))
			{
				SetPhysicsType(rigidbodyType);
			}
		}
		ImGui::EndCombo();
	}

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
}
#endif // EDITOR



void RigidbodyComponent::UpdatePhysicsPosition()
{
	Math::Vector3 scale = m_owner->GetWorldMatrix().GetScale(); // this might cause object to shrink or grow overtime due to floating point precision

	PxMat44 pxMatrix = m_actor->getGlobalPose();
	Math::Matrix matrix = *static_cast<Math::Matrix*>((void*)&pxMatrix);

	m_owner->SetWorldMatrix(Math::Matrix::MakeScale(scale) * matrix);
	
	//float timeSinceTick = PhysXManager::GetInstance()->GetTimeSinceLastSimulationTick();
	//
	//Math::Vector3 translation = PhysXManager::PxVector3ToVector3(m_actor->is<PxRigidDynamic>()->getLinearVelocity()) * timeSinceTick;
	////Math::Vector4 rotation = PhysXManager::PxVector3ToVector3(m_actor->is<PxRigidDynamic>()->getAngularVelocity()) * timeSinceTick;
	//
	////m_owner->GetWorldMatrix().SetPosition(m_owner->GetWorldMatrix().GetPosition() + translation);
	//
	//Math::Vector4 quat(m_actor->getGlobalPose().q.x, m_actor->getGlobalPose().q.y, m_actor->getGlobalPose().q.z, m_actor->getGlobalPose().q.w);
	//
	////m_owner->SetWorldMatrix(m_owner->GetWorldMatrix().MakeRotationFromQuaternion(quat /* + rotation*/)); // spooky
	////m_owner->UpdateTransform();
}

void RigidbodyComponent::SetPhysicsType(PhysXManager::PhysicsType physicsType)
{
	m_physicsType = physicsType;
	if (m_actor)
	{
		m_actor->release();
	}
	m_actor = PhysXManager::GetInstance()->CreateActor(m_physicsType, m_owner->GetWorldMatrix());
}

void RigidbodyComponent::CollectShapes(RigidbodyComponent* collector, GameObject* gameObject)
{
	if (auto shapeComp = gameObject->GetComponentOfType<PhysicsShapeComponent>())
	{
		physx::PxShape* shape = nullptr;

		switch (shapeComp->GetShapeType())
		{
		case PhysXManager::PhysicsShape::Box:
		{
			if (auto meshComp = gameObject->GetComponentOfType<MeshComponent>())
			{
				auto boundingBox = meshComp->GetMesh()->GetBoundingBoxMax() * 0.5f - meshComp->GetMesh()->GetBoundingBoxMin()* 0.5f;
				shape = PhysXManager::GetInstance()->CreateBoxShape(boundingBox * gameObject->GetWorldMatrix().GetScale());
			}
			else
			{
				shape = PhysXManager::GetInstance()->CreateBoxShape(gameObject->GetWorldMatrix().GetScale());
			}
			break;
		}
		case PhysXManager::PhysicsShape::Sphere:
		{
			if (auto meshComp = gameObject->GetComponentOfType<MeshComponent>())
			{
				auto boundingBox = meshComp->GetMesh()->GetBoundingBoxMax() * 0.5f - meshComp->GetMesh()->GetBoundingBoxMin() * 0.5f;
				auto scale = boundingBox * gameObject->GetWorldMatrix().GetScale();
				shape = PhysXManager::GetInstance()->CreateSphereShape(std::max({ scale.x, scale.y, scale.z }));
			}
			else
			{
				auto scale = gameObject->GetWorldMatrix().GetScale();
				shape = PhysXManager::GetInstance()->CreateSphereShape(std::max({ scale.x, scale.y, scale.z }));
			}

			break;
		}
		case PhysXManager::PhysicsShape::TriangleMesh:
		{
			if (auto terrainComp = gameObject->GetComponentOfType<TerrainComponent>())
			{
				shape = PhysXManager::GetInstance()->CreateTriangleShape(terrainComp->GetMesh(), gameObject->GetWorldMatrix().GetScale());
			}
			else if (auto meshComp = gameObject->GetComponentOfType<MeshComponent>())
			{
				shape = PhysXManager::GetInstance()->CreateTriangleShape(meshComp->GetMesh(), gameObject->GetWorldMatrix().GetScale());
			}
			break;
		}
		case PhysXManager::PhysicsShape::ConvexMesh:
		{
			if (auto terrainComp = gameObject->GetComponentOfType<TerrainComponent>())
			{
				shape = PhysXManager::GetInstance()->CreateConvexTriangleShape(terrainComp->GetMesh(), gameObject->GetWorldMatrix().GetScale());
			}
			else if (auto meshComp = gameObject->GetComponentOfType<MeshComponent>())
			{
				shape = PhysXManager::GetInstance()->CreateConvexTriangleShape(meshComp->GetMesh(), gameObject->GetWorldMatrix().GetScale());
			}
			break;
		}
		}
		shape->setLocalPose(PhysXManager::MatrixToPxTransform(gameObject->GetWorldMatrix() * m_owner->GetWorldMatrix().GetInverse()));


		PxFilterData filter;
		filter.word0 = 1;
		shape->setQueryFilterData(filter);

		collector->GetActor()->attachShape(*shape);
	}

	for (auto& child : gameObject->GetChildren())
	{
		if (auto jointComponent = child->GetComponentOfType<JointComponent>() )
		{
			PxJoint* joint = nullptr;
			switch (jointComponent->GetJointType())
			{
			case JointComponent::JointType::Revolute:
				joint = PxRevoluteJointCreate(
					*PhysXManager::GetInstance()->GetPhysics(),
					child->GetComponentOfType<RigidbodyComponent>()->GetActor(),
					PxTransform(PxIdentity),
					m_actor,
					PhysXManager::MatrixToPxTransform(child->GetLocalMatrix() * Math::Matrix::MakeScale(child->GetParent()->GetWorldMatrix().GetScale()))
				);
				break;
							
			case JointComponent::JointType::Piston:
				joint = PxDistanceJointCreate(
					*PhysXManager::GetInstance()->GetPhysics(),
					child->GetComponentOfType<RigidbodyComponent>()->GetActor(),
					PxTransform(PxIdentity),
					m_actor,
					PhysXManager::MatrixToPxTransform(child->GetLocalMatrix() * Math::Matrix::MakeScale(child->GetParent()->GetWorldMatrix().GetScale()))
				);
				break;
			case JointComponent::JointType::Spring:
				joint = PxDistanceJointCreate(
					*PhysXManager::GetInstance()->GetPhysics(),
					child->GetComponentOfType<RigidbodyComponent>()->GetActor(),
					PxTransform(PxIdentity),
					m_actor,
					PhysXManager::MatrixToPxTransform(child->GetLocalMatrix() * Math::Matrix::MakeScale(child->GetParent()->GetWorldMatrix().GetScale()))
				);
				static_cast<PxDistanceJoint*>(joint)->setDistanceJointFlag(PxDistanceJointFlag::eSPRING_ENABLED, true);
				static_cast<PxDistanceJoint*>(joint)->setStiffness(10000.0f);
				static_cast<PxDistanceJoint*>(joint)->setDamping(1.0f);

				break;
			}			
			m_runtimeJoints.push_back(joint);
		}
		if (auto shapeComp = child->GetComponentOfType<RigidbodyComponent>())
		{
			continue;
		}

		CollectShapes(collector, child);

	}
	if (auto rigidActor = GetActor()->is<PxRigidDynamic>())
	{
		PxRigidBodyExt::setMassAndUpdateInertia(*rigidActor, rigidActor->getMass());
	}
}
