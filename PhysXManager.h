#pragma once
#include "pch.h"
#include <PxPhysicsAPI.h>
#include <foundation/PxFoundation.h>
#include <extensions/PxDefaultErrorCallback.h>
#include <extensions/PxDefaultAllocator.h>
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
	};

	enum PhysicsShape
	{
		TriangleMesh,
		Box,
		Sphere,
	};

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
	PxShape* CreateTriangleShape(const Mesh& mesh);

	void DetachShapesFromActor(PxRigidActor* actor);

	physx::PxTransform MatrixToPxTransform(const Math::Matrix& transform);

private:
	PhysXManager();
	~PhysXManager();


	PxDefaultAllocator m_allocator;
	PxDefaultErrorCallback m_errorCallback;
	PxFoundation* m_foundation					= NULL;
	PxPhysics* m_physics						= NULL;
	PxScene* m_scene							= NULL;
	PxDefaultCpuDispatcher* m_dispatcher		= NULL;
	PxCooking* m_cooking;
	physx::PxMaterial* m_defaultMaterial;
	PxShapeFlags m_defaultShapeFlag;
	 
};
