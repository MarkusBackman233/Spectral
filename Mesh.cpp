#include "Mesh.h"
#include "Vector2.h"
#include <iostream>
#include <string>
#include <vector>
#include "Vector3.h"
#include "Vector2.h"
#include "Triangle.h"
#include <fstream>
#include <strstream>
#include "Mesh.h"
#include "iRender.h"
#include "TextureManager.h"
#include "MaterialManager.h"

Mesh::Mesh()
	: m_cachedTriangleMesh(nullptr)
{
	m_material = MaterialManager::GetInstance()->GetDefaultMaterial();
}


void Mesh::SetName(const std::string& name)
{
	m_meshName = name;
}

std::string& Mesh::GetName()
{
	return m_meshName;
}

void Mesh::CalculateBoundingBox()
{
	m_minBounds = Math::Vector3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	m_maxBounds = Math::Vector3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

	for (const auto& vertex : vertexes)
	{
		if (vertex.position.x < m_minBounds.x) m_minBounds.x = vertex.position.x;
		if (vertex.position.y < m_minBounds.y) m_minBounds.y = vertex.position.y;
		if (vertex.position.z < m_minBounds.z) m_minBounds.z = vertex.position.z;

		if (vertex.position.x > m_maxBounds.x) m_maxBounds.x = vertex.position.x;
		if (vertex.position.y > m_maxBounds.y) m_maxBounds.y = vertex.position.y;
		if (vertex.position.z > m_maxBounds.z) m_maxBounds.z = vertex.position.z;
	}
}


PxTriangleMesh* Mesh::CreatePhysxTriangleMesh() const
{
	const bool skipMeshCleanup = false;
	const bool skipEdgeData = false;
	const bool inserted = false;

	const int numTrisPerLeaf = 15;

	auto physXManager = PhysXManager::GetInstance();

	PxVec3* vertices = new PxVec3[vertexes.size()];
	PxU32* indices = new PxU32[indices32.size()];

	for (int i = 0; i < vertexes.size(); i++)
	{
		vertices[i].x = -vertexes[i].position.x;
		vertices[i].y =  vertexes[i].position.y;
		vertices[i].z =  vertexes[i].position.z;
	}

	memcpy(indices, indices32.data(), sizeof(unsigned int) * indices32.size());

	PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = (PxU32)vertexes.size();
	meshDesc.points.data = vertices;
	meshDesc.points.stride = sizeof(PxVec3);
	meshDesc.triangles.count = (PxU32)triangles.size();
	meshDesc.triangles.data = indices;
	meshDesc.triangles.stride = 3 * sizeof(PxU32);
	PxCookingParams params = physXManager->GetCooking()->getParams();

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

	physXManager->GetCooking()->setParams(params);

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
		triangleMesh =  physXManager->GetCooking()->createTriangleMesh(meshDesc, physXManager->GetPhysics()->getPhysicsInsertionCallback());
	}
	else
	{
		PxDefaultMemoryOutputStream outBuffer;
		physXManager->GetCooking()->cookTriangleMesh(meshDesc, outBuffer);

		PxDefaultMemoryInputData stream(outBuffer.getData(), outBuffer.getSize());
		triangleMesh = physXManager->GetPhysics()->createTriangleMesh(stream);
	}

	printf("\t -----------------------------------------------\n");
	printf("\t Create triangle mesh with %i triangles: \n", (int)triangles.size());
	delete[] vertices;
	delete[] indices;
	return triangleMesh;

}

