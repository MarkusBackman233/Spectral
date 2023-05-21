#pragma once
#include "Vector3.h"
#include <vector>
#include "Triangle.h"
#include <SDL.h>
#include "PhysXManager.h"

class Mesh
{
public:
	std::vector<Triangle> triangles;
	std::vector<uint32_t> indices32;

	int vertexCount = 0;

	PxTriangleMesh* CreatePhysxTriangleMesh();

	PxTriangleMesh* m_physXMeshCollider = NULL;

	std::string filename;
	std::vector<Math::Vector3> vertexes;

};

