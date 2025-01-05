#pragma once
#include "pch.h"
#include <PxPhysicsAPI.h>
#include <foundation/PxFoundation.h>
#include <extensions/PxDefaultErrorCallback.h>
#include <extensions/PxDefaultAllocator.h>
#include <vehicle2/PxVehicleAPI.h>

#include <PxBaseMaterial.h>
#include <cooking/PxCooking.h>
#include "Vector3.h"
#include "Matrix.h"
class Mesh;
using namespace physx;

class PhysXManager
{
public:

	enum PhysicsType
	{
		StaticActor,
		DynamicActor,
		VehicleActor,
	};

	enum PhysicsShape
	{
		Box,
		Sphere,
		TriangleMesh,
		ConvexMesh,
	};

	static std::unordered_map<PhysicsShape, std::string> PhysicsShapeToString;


	enum CollisionGroup
	{
		GROUP_DEFAULT,
		GROUP_DYNAMIC,
		GROUP_STATIC,
	};

	static PhysXManager* GetInstance() {
		static PhysXManager instance;
		return &instance;
	}

	PxScene* GetScene() { return m_scene; }
	PxPhysics* GetPhysics() { return m_physics; }
	PxCooking* GetCooking() { return m_cooking; }

	void TickSimulation(float deltaTime);

	PxRigidActor* CreateActor(PhysicsType type, const Math::Matrix& transform);
	PxRigidActor* CreateActor(PxRigidActor* actor);
	PxShape* CreateBoxShape(const Math::Vector3& boxScale);
	PxShape* CreateSphereShape(float radius);
	PxShape* CreateTriangleShape(const std::shared_ptr<Mesh>& mesh, const Math::Vector3& scale = Math::Vector3());

	PxShape* CreateConvexTriangleShape(const std::shared_ptr<Mesh>& mesh, const Math::Vector3& scale = Math::Vector3());

	PxTriangleMesh* CreatePhysxTriangleMesh(Mesh* mesh) const;
	PxConvexMesh* CreateConvexShape(Mesh* mesh);

	void DetachShapesFromActor(PxRigidActor* actor);

	physx::PxMaterial* GetDefaultMaterial() { return m_defaultMaterial; }

	static physx::PxTransform MatrixToPxTransform(const Math::Matrix& transform);
	static Math::Vector3 PxVector3ToVector3(const physx::PxExtendedVec3& vector);
	static Math::Vector3 PxVector3ToVector3(const physx::PxVec3& vector);
	static physx::PxVec3 Vector3ToPxVector3(const Math::Vector3& vector);

	float GetLastSimulationTick() const;
	float GetTimeSinceLastSimulationTick() const;

	PxControllerManager* GetControllerManager();

private:
	PhysXManager();
	~PhysXManager();
	PxDefaultAllocator m_allocator;
	PxDefaultErrorCallback m_errorCallback;
	PxFoundation* m_foundation					= nullptr;
	PxPhysics* m_physics						= nullptr;
	PxScene* m_scene							= nullptr;
	PxDefaultCpuDispatcher* m_dispatcher		= nullptr;
	PxControllerManager* m_controllerManager	= nullptr;
	PxCooking* m_cooking;
	physx::PxMaterial* m_defaultMaterial;
	PxShapeFlags m_defaultShapeFlag;


	float m_lastSimulationTick;
	float m_accumulatedTime;
};
