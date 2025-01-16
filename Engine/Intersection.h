#pragma once
#include "Vector3.h"
#include "Matrix.h"
class Mesh;

namespace Spectral
{
class Intersection
{
public:
	static bool BoundingSphere(const Math::Matrix& matrix, float radius, const Math::Vector3& origin, const Math::Vector3& direction, float& distance);

	static bool LocalBoundingBox(const Math::Vector3& minBounding,const Math::Vector3& maxBounding, const Math::Vector3& origin, const Math::Vector3& direction);
	static bool LocalBoundingBox(Mesh* mesh, const Math::Vector3& origin, const Math::Vector3& direction);

	static bool LocalTriangle(const Math::Vector3& origin, const Math::Vector3& direction, const Math::Vector3& p1, const Math::Vector3& p2, const Math::Vector3& p3, float& distance);

	static bool MeshTriangles(Mesh* mesh, const Math::Matrix& matrix, const Math::Vector3& origin, const Math::Vector3& direction, float& distance);
};
}