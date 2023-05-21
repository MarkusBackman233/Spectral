#pragma once
#include <PxPhysicsAPI.h>
#include <foundation/PxFoundation.h>
#include <extensions/PxDefaultErrorCallback.h>
#include <extensions/PxDefaultAllocator.h>
#include <PxBaseMaterial.h>
#include <iostream>
#include <cooking/PxCooking.h>


using namespace physx;

class PhysXManager
{
public:

	enum CollisionGroup
	{
		GROUP_DEFAULT = 0,
		GROUP_DYNAMIC = 1,
		GROUP_STATIC = 2,
	};

	static PhysXManager& GetInstance() {
		static PhysXManager instance;
		return instance;
	}

	PxScene* GetScene() { return m_scene; }
	PxPhysics* GetPhysics() { return m_physics; }
	PxCooking* GetCooking() { return m_cooking; }

	void TickSimulation(float deltaTime);


	physx::PxRigidDynamic* m_player;

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


};
