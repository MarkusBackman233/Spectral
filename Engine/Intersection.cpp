#include "Intersection.h"
#include "DirectXMath.h"
#include "DirectXCollision.h"
#include "DxMathUtils.h"
#include "Mesh.h"
#include "iRender.h"
using namespace Spectral;

bool Spectral::Intersection::BoundingSphere(const Math::Matrix& matrix, float radius, const Math::Vector3& origin, const Math::Vector3& direction, float& distance)
{
    DirectX::BoundingSphere sphere;
    sphere.Center = DirectX::XMFLOAT3(matrix.GetPosition().x, matrix.GetPosition().y, matrix.GetPosition().z);
    sphere.Radius = radius;
    return sphere.Intersects(DxMathUtils::ToDx(origin), DxMathUtils::ToDx(direction), distance);
}

bool Intersection::LocalBoundingBox(const Math::Vector3& minBounding, const Math::Vector3& maxBounding, const Math::Vector3& origin, const Math::Vector3& direction)
{
    DirectX::BoundingBox boundingBox;
    DirectX::BoundingBox::CreateFromPoints(boundingBox, DxMathUtils::ToDx(minBounding), DxMathUtils::ToDx(maxBounding));
    float distance = 0.0f;
    return boundingBox.Intersects(DxMathUtils::ToDx(origin), DxMathUtils::ToDx(direction), distance);
}

bool Intersection::LocalBoundingBox(Mesh* mesh, const Math::Vector3& origin, const Math::Vector3& direction)
{
    return LocalBoundingBox(mesh->GetBoundingBoxMin(), mesh->GetBoundingBoxMax(), origin, direction);
}

bool Intersection::LocalTriangle(const Math::Vector3& origin, const Math::Vector3& direction, const Math::Vector3& p1, const Math::Vector3& p2, const Math::Vector3& p3, float& distance)
{
    return DirectX::TriangleTests::Intersects(DxMathUtils::ToDx(origin), DxMathUtils::ToDx(direction), DxMathUtils::ToDx(p1), DxMathUtils::ToDx(p2), DxMathUtils::ToDx(p3), distance);
}

bool Intersection::MeshTriangles(Mesh* mesh, const Math::Matrix& matrix, const Math::Vector3& origin, const Math::Vector3& direction, float& distance)
{
    Math::Matrix inverseObjectMatrix = matrix.GetInverse();
    Math::Vector3 localRayOrigin = origin.Transform(inverseObjectMatrix);
    Math::Vector3 localRayDirection = direction.TransformNormal(inverseObjectMatrix).GetNormal();
    if (!Intersection::LocalBoundingBox(mesh, localRayOrigin, localRayDirection))
        return false;

    distance = FLT_MAX;
    bool hasHitTriangle = false;
    for (size_t i = 0; i < mesh->indices32.size(); i += 3)
    {
        Math::Vector3 p1 = mesh->vertexes[mesh->indices32[i]].position;
        Math::Vector3 p2 = mesh->vertexes[mesh->indices32[i + 1]].position;
        Math::Vector3 p3 = mesh->vertexes[mesh->indices32[i + 2]].position;
        float localDistance = 0.0f;
        if (Intersection::LocalTriangle(localRayOrigin, localRayDirection, p1, p2, p3, localDistance))
        {
            auto localHitPos = localRayOrigin + localRayDirection * localDistance;

            auto worldHitPos = localHitPos.Transform(matrix);
            distance = std::min(distance, (Math::Vector3(worldHitPos.x, worldHitPos.y, worldHitPos.z) - origin).LengthSquared());
            hasHitTriangle = true;
        }
    }

    distance = sqrt(distance);

    return hasHitTriangle;
}

bool Spectral::Intersection::MeshTrianglesLocal(Mesh* mesh, const Math::Vector3& origin, const Math::Vector3& direction, float& distance)
{
    distance = FLT_MAX;
    bool hasHitTriangle = false;
    for (size_t i = 0; i < mesh->indices32.size(); i += 3)
    {
        Math::Vector3 p1 = mesh->vertexes[mesh->indices32[i]].position;
        Math::Vector3 p2 = mesh->vertexes[mesh->indices32[i + 1]].position;
        Math::Vector3 p3 = mesh->vertexes[mesh->indices32[i + 2]].position;
        float d = 0.0f;
        if (Intersection::LocalTriangle(origin, direction, p1, p2, p3, d))
        {
            distance = std::min(d, distance);
            hasHitTriangle = true;
        }
    }

    return hasHitTriangle;
}
