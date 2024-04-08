#include "PhysXManager.h"
#include <iostream>
#include "Mesh.h"
#include "Logger.h"

PhysXManager::PhysXManager()
{
	LogMessage("PhysXManager initialized");

	m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_allocator, m_errorCallback);
	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, PxTolerancesScale());

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

	m_defaultMaterial = m_physics->createMaterial(0.5f, 0.5f, 0.1f);
#ifdef EDITOR
	m_defaultShapeFlag = PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE;
#else
	m_defaultShapeFlag = PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE;
#endif // EDITOR
}


PhysXManager::~PhysXManager()
{

	m_scene->release();
	m_physics->release();
	m_foundation->release();
}

void PhysXManager::DetachShapesFromActor(PxRigidActor* actor)
{
	PxU32 numShapes = actor->getNbShapes();
	PxShape** shapes = new PxShape*[numShapes];
	actor->getShapes(shapes, numShapes);
	for (PxU32 i = 0; i < numShapes; i++)
	{
		actor->detachShape(*shapes[i]);
	}
	delete[] shapes;
}

physx::PxTransform PhysXManager::MatrixToPxTransform(const Math::Matrix& transform)
{
	PxVec3 pos(transform.m_matrix[3][0], transform.m_matrix[3][1], transform.m_matrix[3][2]);
	
	auto quat = transform.MatrixToQuaternion();

	auto returnMat = physx::PxTransform(pos, PxQuat(-quat.x, -quat.y, -quat.z, quat.w));

	Math::Vector3 right(transform.m_matrix[0][0], transform.m_matrix[0][1], transform.m_matrix[0][2], transform.m_matrix[0][3]);
	Math::Vector3 up(transform.m_matrix[1][0], transform.m_matrix[1][1], transform.m_matrix[1][2], transform.m_matrix[1][3]);
	Math::Vector3 front(transform.m_matrix[2][0], transform.m_matrix[2][1], transform.m_matrix[2][2], transform.m_matrix[2][3]);

	PxVec4 scale(sqrt(right.Length()), sqrt(up.Length()), sqrt(front.Length()),1.0f);

	PxMat44 asd(returnMat);
	asd.scale(scale);
	return PxTransform(asd);
}



void PhysXManager::TickSimulation(float deltaTime)
{
	static float accumulatedTime = 0.0f;
	const float fixedTimeStep = 1.0f / 60.0f;

	accumulatedTime += deltaTime;

	while (accumulatedTime >= fixedTimeStep)
	{
		m_scene->simulate(fixedTimeStep);
		m_scene->fetchResults(true);
		accumulatedTime -= fixedTimeStep;
	}
}

PxRigidActor* PhysXManager::CreateActor(PhysicsType type, const Math::Matrix& transform)
{
	auto physXManager = PhysXManager::GetInstance();
	PxRigidActor* newActor = nullptr;
	
	if (type == PhysicsType::DynamicActor)
	{
		newActor = physXManager->GetPhysics()->createRigidDynamic(MatrixToPxTransform(transform));
	}
	else if(type == PhysicsType::StaticActor)
	{
		newActor = physXManager->GetPhysics()->createRigidStatic(MatrixToPxTransform(transform));
	}
	physXManager->GetScene()->addActor(*newActor);
	return newActor;
}

PxRigidActor* PhysXManager::CreateActor(PxRigidActor* actor)
{
	PxRigidActor* newActor = nullptr;
	if (actor->is<PxRigidDynamic>())
	{
		newActor =
			PxCloneDynamic(*PhysXManager::GetInstance()->GetPhysics(), actor->getGlobalPose(), *actor->is<PxRigidDynamic>());
	}
	else
	{
		newActor =
			PxCloneStatic(*PhysXManager::GetInstance()->GetPhysics(), actor->getGlobalPose(), *actor->is<PxRigidStatic>());
	}
	PhysXManager::GetInstance()->GetScene()->addActor(*newActor);
	return newActor;
}

PxShape* PhysXManager::CreateBoxShape(const Math::Vector3& boxScale)
{
	PxBoxGeometry boxGeometry(PxVec3(boxScale.x, boxScale.y, boxScale.z));
	return GetPhysics()->createShape(boxGeometry, &m_defaultMaterial, true, m_defaultShapeFlag);
}

PxShape* PhysXManager::CreateSphereShape(float radius)
{
	PxSphereGeometry geometry(radius);
	return GetPhysics()->createShape(geometry, &m_defaultMaterial, true, m_defaultShapeFlag);
}

PxShape* PhysXManager::CreateTriangleShape(const Mesh& mesh)
{

	PxTriangleMeshGeometry triGeom;
	triGeom.triangleMesh = mesh.CreatePhysxTriangleMesh();
	return GetPhysics()->createShape(triGeom, &m_defaultMaterial, 1, true, m_defaultShapeFlag);
}
//PxShape* PhysXManager::CreateShape(PhysicsShape shape)
//{
//	PxShapeFlags shapeFlags = PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE;
//	auto physXManager = PhysXManager::GetInstance();
//	physx::PxMaterial* defaultMaterial = physXManager->GetPhysics()->createMaterial(0.5f, 0.5f, 0.1f);
//
//	switch (shape)
//	{
//	case PhysicsShape::TriangleMesh:
//	{
//		//PxTriangleMeshGeometry triGeom;
//		//triGeom.triangleMesh = m_mesh->CreatePhysxTriangleMesh();
//		//PxShape* meshShape = physXManager->GetPhysics()->createShape(triGeom, &defaultMaterial, 1, true, shapeFlags);
//		//m_actor->attachShape(*meshShape);
//		break;
//	}
//	case PhysicsShape::Box:
//	{
//		PxBoxGeometry boxGeometry(physx::PxVec3((m_maxBounds.x - m_minBounds.x) * 0.5f, (m_maxBounds.y - m_minBounds.y) * 0.5f, (m_maxBounds.z - m_minBounds.z) * 0.5f));
//		PxShape* boxShape = physXManager->GetPhysics()->createShape(boxGeometry, *defaultMaterial, true, shapeFlags);
//		//m_actor->attachShape(*boxShape);
//		break;
//	}
//	case PhysicsShape::Sphere:
//	{
//		PxSphereGeometry geometry(1.0f);
//		PxShape* shape = physXManager->GetPhysics()->createShape(geometry, *defaultMaterial, true, shapeFlags);
//		//m_actor->attachShape(*shape);
//		break;
//	}
//	}
//}