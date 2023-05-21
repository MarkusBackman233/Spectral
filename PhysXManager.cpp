#include "PhysXManager.h"


PhysXManager::PhysXManager()
{
	std::cout << "PhysXManager initialized" << std::endl;

	// Create the foundation
	m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_allocator, m_errorCallback);

	// Create the physics object

	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, PxTolerancesScale());
	// Create the scene

	PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	m_dispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = m_dispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	m_scene = m_physics->createScene(sceneDesc);
	m_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	m_scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);

	PxCookingParams cookingParams = PxCookingParams(physx::PxTolerancesScale());
	cookingParams.meshWeldTolerance = 0.01f;
	cookingParams.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;


	m_cooking = PxCreateCooking(PX_PHYSICS_VERSION, m_physics->getFoundation(), cookingParams);
}


PhysXManager::~PhysXManager()
{

	m_scene->release();
	m_physics->release();
	m_foundation->release();
}



void PhysXManager::TickSimulation(float deltaTime)
{
	static float accumulatedTime = 0.0f;
	const float fixedTimeStep = 1.0f / 60.0f; // the desired fixed time step

	// accumulate the elapsed time
	accumulatedTime += deltaTime;

	// simulate as many fixed time steps as needed to catch up
	while (accumulatedTime >= fixedTimeStep)
	{
		m_scene->simulate(fixedTimeStep);
		m_scene->fetchResults(true);
		accumulatedTime -= fixedTimeStep;
	}

	//m_scene->simulate(1.0f / 60.0f);
	//m_scene->fetchResults(true);
}
