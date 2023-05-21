#include "Mesh.h"
#include "Vector2.h"
#include <iostream>
#include <string>



PxTriangleMesh* Mesh::CreatePhysxTriangleMesh()
{
	const bool skipMeshCleanup = false;
	const bool skipEdgeData = false;
	const bool inserted = false;

	const int numTrisPerLeaf = 15;

	PhysXManager& physXManager = PhysXManager::GetInstance();



	PxVec3* vertices = new PxVec3[vertexes.size()];
	PxU32* indices = new PxU32[triangles.size() * 3];


	for (int i = 0; i < vertexes.size(); i++)
	{
		vertices[i].x = -vertexes[i].x;
		vertices[i].y = vertexes[i].y;
		vertices[i].z = vertexes[i].z;
	}

	for (int i = 0; i < indices32.size(); i++)
	{
		indices[i] = indices32[i];
	}

	PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = (PxU32)vertexes.size();
	meshDesc.points.data = vertices;
	meshDesc.points.stride = sizeof(PxVec3);
	meshDesc.triangles.count = (PxU32)triangles.size();
	meshDesc.triangles.data = indices;
	meshDesc.triangles.stride = 3 * sizeof(PxU32);

	PxCookingParams params = physXManager.GetCooking()->getParams();

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

	physXManager.GetCooking()->setParams(params);

#if defined(PX_CHECKED) || defined(PX_DEBUG)
	// If DISABLE_CLEAN_MESH is set, the mesh is not cleaned during the cooking. 
	// We should check the validity of provided triangles in debug/checked builds though.
	if (skipMeshCleanup)
	{
		PX_ASSERT(gCooking->validateTriangleMesh(meshDesc));
	}
#endif // DEBUG



	PxU32 meshSize = 0;

	// The cooked mesh may either be saved to a stream for later loading, or inserted directly into PxPhysics.
	if (inserted)
	{
		m_physXMeshCollider = physXManager.GetCooking()->createTriangleMesh(meshDesc, physXManager.GetPhysics()->getPhysicsInsertionCallback());
	}
	else
	{
		PxDefaultMemoryOutputStream outBuffer;
		physXManager.GetCooking()->cookTriangleMesh(meshDesc, outBuffer);

		PxDefaultMemoryInputData stream(outBuffer.getData(), outBuffer.getSize());
		m_physXMeshCollider = physXManager.GetPhysics()->createTriangleMesh(stream);

		meshSize = outBuffer.getSize();
	}

	printf("\t -----------------------------------------------\n");
	printf("\t Create triangle mesh with %i triangles: \n", (int)triangles.size());
	inserted ? printf("\t\t Mesh inserted on\n") : printf("\t\t Mesh inserted off\n");
	!skipEdgeData ? printf("\t\t Precompute edge data on\n") : printf("\t\t Precompute edge data off\n");
	!skipMeshCleanup ? printf("\t\t Mesh cleanup on\n") : printf("\t\t Mesh cleanup off\n");
	printf("\t\t Num triangles per leaf: %i \n", (int)numTrisPerLeaf);
	if (!inserted)
	{
		printf("\t Mesh size: %d \n", meshSize);
	}
	return m_physXMeshCollider;



	delete[] vertices;
	delete[] indices;

}
