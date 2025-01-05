#include "PhysXManager.h"
#include "Mesh.h"
#include "Logger.h"
#include "iRender.h"
#ifdef EDITOR
#include "Editor.h"
#endif

std::unordered_map<PhysXManager::PhysicsShape, std::string> PhysXManager::PhysicsShapeToString = {
	{PhysicsShape::Box, "Box" },
	{PhysicsShape::Sphere, "Sphere" },
	{PhysicsShape::TriangleMesh, "Triangle Mesh" },
	{PhysicsShape::ConvexMesh, "Convex Mesh" },
};

PhysXManager::PhysXManager()
	: m_lastSimulationTick(0.0f)
	, m_accumulatedTime(0.0f)
{
	Logger::Info("PhysXManager initialized");

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

	m_controllerManager = PxCreateControllerManager(*m_scene);

	m_defaultMaterial = m_physics->createMaterial(0.8f, 0.5f, 0.1f);
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
	auto scaledMatrix = transform;
	auto scale = scaledMatrix.GetScale();
	scale.x = 1.0f / scale.x;
	scale.y = 1.0f / scale.y;
	scale.z = 1.0f / scale.z;
	scaledMatrix = Math::Matrix::MakeScale(scale) * scaledMatrix;
	
	PxMat44 pxMatrix = *static_cast<PxMat44*>((void*)&scaledMatrix);
	return PxTransform(pxMatrix);
}

Math::Vector3 PhysXManager::PxVector3ToVector3(const physx::PxExtendedVec3& vector)
{
	return Math::Vector3(static_cast<float>(vector.x), static_cast<float>(vector.y), static_cast<float>(vector.z));
}

Math::Vector3 PhysXManager::PxVector3ToVector3(const physx::PxVec3& vector)
{
	return Math::Vector3(vector.x, vector.y, vector.z);
}

physx::PxVec3 PhysXManager::Vector3ToPxVector3(const Math::Vector3& vector)
{
	return physx::PxVec3(vector.x, vector.y, vector.z);
}

float PhysXManager::GetLastSimulationTick() const
{
	return m_lastSimulationTick;
}

float PhysXManager::GetTimeSinceLastSimulationTick() const
{
	return m_accumulatedTime - m_lastSimulationTick;
}

PxControllerManager* PhysXManager::GetControllerManager()
{
	return m_controllerManager;
}

void PhysXManager::TickSimulation(float deltaTime)
{
	static float accumulatedTime = 0.0f;
	const float fixedTimeStep = 1.0f / 120.0f;
	accumulatedTime += deltaTime;
	m_accumulatedTime += deltaTime;
	while (accumulatedTime >= fixedTimeStep)
	{
		m_scene->simulate(fixedTimeStep);
		m_scene->fetchResults(true);
		accumulatedTime -= fixedTimeStep;
		m_lastSimulationTick = m_accumulatedTime;
	}

#ifdef EDITOR
	if (Editor::GetInstance()->IsStarted() == false)
	{
		const PxRenderBuffer& rb = PhysXManager::GetInstance()->GetScene()->getRenderBuffer();
		Math::Vector4 color(0.27f, 0.69f, 0.2f,1.0f);
		for (PxU32 i = 0; i < rb.getNbLines(); i++)
		{
			Render::DrawLine(PhysXManager::PxVector3ToVector3(rb.getLines()[i].pos0), PhysXManager::PxVector3ToVector3(rb.getLines()[i].pos1), color);
		}
	}
#endif
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

PxShape* PhysXManager::CreateTriangleShape(const std::shared_ptr<Mesh>& mesh, const Math::Vector3& scale)
{
	PxTriangleMeshGeometry triGeom;
	triGeom.triangleMesh = CreatePhysxTriangleMesh(mesh.get());
	triGeom.scale.scale.x = scale.x;
	triGeom.scale.scale.y = scale.y;
	triGeom.scale.scale.z = scale.z;
	auto shape = GetPhysics()->createShape(triGeom, &m_defaultMaterial, 1, true, m_defaultShapeFlag);
	return shape;
}

PxShape* PhysXManager::CreateConvexTriangleShape(const std::shared_ptr<Mesh>& mesh, const Math::Vector3& scale)
{
	PxConvexMeshGeometry convexTriGeom;
	convexTriGeom.convexMesh = CreateConvexShape(mesh.get());
	convexTriGeom.scale.scale.x = scale.x;
	convexTriGeom.scale.scale.y = scale.y;
	convexTriGeom.scale.scale.z = scale.z;
	auto shape =  GetPhysics()->createShape(convexTriGeom, &m_defaultMaterial, 1, true, m_defaultShapeFlag);
	return shape;
}


PxTriangleMesh* PhysXManager::CreatePhysxTriangleMesh(Mesh* mesh) const
{
	const bool skipMeshCleanup = false;
	const bool skipEdgeData = false;
	const bool inserted = false;

	const int numTrisPerLeaf = 15;


	std::vector<PxVec3> vertices(mesh->vertexes.size());

	for (int i = 0; i < mesh->vertexes.size(); i++)
	{
		vertices[i].x = mesh->vertexes[i].position.x;
		vertices[i].y = mesh->vertexes[i].position.y;
		vertices[i].z = mesh->vertexes[i].position.z;
	}
	std::vector<uint32_t> flippedIndices = mesh->indices32;

	// Assuming each triangle has 3 indices, swap the second and third index in each triangle
	for (size_t i = 0; i < flippedIndices.size(); i += 3)
	{
		std::swap(flippedIndices[i + 1], flippedIndices[i + 2]);
	}

	PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = (PxU32)mesh->vertexes.size();
	meshDesc.points.data = vertices.data();
	meshDesc.points.stride = sizeof(PxVec3);
	meshDesc.triangles.count = (PxU32)mesh->indices32.size() / 3;
	meshDesc.triangles.data = flippedIndices.data();
	meshDesc.triangles.stride = 3 * sizeof(uint32_t);
	PxCookingParams params = m_cooking->getParams();

	// Create BVH34 midphase
	params.midphaseDesc = PxMeshMidPhase::eBVH34;
	params.suppressTriangleMeshRemapTable = true;

	// If DISABLE_CLEAN_MESH is set, the mesh is not cleaned during the cooking. The input mesh must be valid. 
	// The following conditions are true for a valid triangle mesh :
	//  1. There are no duplicate vertices(within specified vertexWeldTolerance.See PxCookingParams::meshWeldTolerance)
	//  2. There are no large triangles(within specified PxTolerancesScale.)
	// It is recommended to run a separate validation check in debug/checked builds, see below.

	if (!skipMeshCleanup)
		params.meshPreprocessParams &= ~static_cast<PxMeshPreprocessingFlags>(PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH);
	else
		params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;

	// If eDISABLE_ACTIVE_EDGES_PRECOMPUTE is set, the cooking does not compute the active (convex) edges, and instead 
	// marks all edges as active. This makes cooking faster but can slow down contact generation. This flag may change 
	// the collision behavior, as all edges of the triangle mesh will now be considered active.
	if (!skipEdgeData)
		params.meshPreprocessParams &= ~static_cast<PxMeshPreprocessingFlags>(PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE);
	else
		params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;

	// Cooking mesh with less triangles per leaf produces larger meshes with better runtime performance
	// and worse cooking performance. Cooking time is better when more triangles per leaf are used.
	params.midphaseDesc.mBVH34Desc.numPrimsPerLeaf = numTrisPerLeaf;
	m_cooking->setParams(params);

#if defined(PX_CHECKED) || defined(PX_DEBUG)
	// If DISABLE_CLEAN_MESH is set, the mesh is not cleaned during the cooking. 
	// We should check the validity of provided triangles in debug/checked builds though.
	if (skipMeshCleanup)
	{
		PX_ASSERT(gCooking->validateTriangleMesh(meshDesc));
	}
#endif // DEBUG

	// The cooked mesh may either be saved to a stream for later loading, or inserted directly into PxPhysics.
	PxTriangleMesh* triangleMesh = nullptr; // I need to cache this 

	if (inserted)
	{
		triangleMesh = m_cooking->createTriangleMesh(meshDesc, m_physics->getPhysicsInsertionCallback());
	}
	else
	{
		PxDefaultMemoryOutputStream outBuffer;
		m_cooking->cookTriangleMesh(meshDesc, outBuffer);

		PxDefaultMemoryInputData stream(outBuffer.getData(), outBuffer.getSize());
		triangleMesh = m_physics->createTriangleMesh(stream);
	}

	Logger::Info("\t -----------------------------------------------\n");
	Logger::Info("\t Create triangle mesh with %i triangles: \n" +  (int)meshDesc.triangles.count);

	return triangleMesh;

}

PxConvexMesh* PhysXManager::CreateConvexShape(Mesh* mesh)
{
	std::vector<PxVec3> vertices(mesh->vertexes.size());

	for (int i = 0; i < mesh->vertexes.size(); i++)
	{
		vertices[i].x = mesh->vertexes[i].position.x;
		vertices[i].y = mesh->vertexes[i].position.y;
		vertices[i].z = mesh->vertexes[i].position.z;
	}
	PxConvexMeshDesc convexMeshDesc;
	convexMeshDesc.points.count = (PxU32)mesh->vertexes.size();
	convexMeshDesc.points.stride = sizeof(PxVec3);
	convexMeshDesc.points.data = vertices.data();

	convexMeshDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

	// cook mesh to output stream

	PxDefaultMemoryOutputStream cookedMeshOutput;
	PxConvexMeshCookingResult::Enum cookingResult;
	if (!PhysXManager::GetInstance()->GetCooking()->cookConvexMesh(convexMeshDesc, cookedMeshOutput, &cookingResult))
		throw std::runtime_error::runtime_error("Could not cook convex mesh");

	// use output as input to convex mesh
	PxDefaultMemoryInputData cookedMeshInput(cookedMeshOutput.getData(), cookedMeshOutput.getSize());
	PxConvexMesh* convexMesh = PhysXManager::GetInstance()->GetPhysics()->createConvexMesh(cookedMeshInput);

	return convexMesh;
}