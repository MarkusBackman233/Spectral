#include "RaycastManager.h"


bool RaycastManager::Raycast(Math::Vector3 rayOrigin,
    Math::Vector3 rayDirection,
    RaycastHit& hit,
    Mesh* physicsMesh)
{

    //float closestHitDistance = 10000.0f;
    //int closestTriangleIterator = -1;
    //int i = -1;
    //for (const auto& triangle : physicsMesh->triangles) {
    //    i++;
    //    const float EPSILON = 0.0000001;
    //    Math::Vector3 vertex0 = triangle.m_vertex[0];
    //    Math::Vector3 vertex1 = triangle.m_vertex[1];
    //    Math::Vector3 vertex2 = triangle.m_vertex[2];
    //    Math::Vector3 edge1, edge2, h, s, q;
    //    float a, f, u, v;
    //    edge1 = vertex1 - vertex0;
    //    edge2 = vertex2 - vertex0;
    //    h = rayDirection.Cross(edge2);
    //    a = edge1.Dot(h);
    //    if (a > -EPSILON && a < EPSILON)
    //        continue;    // This ray is parallel to this triangle.
    //    f = 1.0 / a;
    //    s = rayOrigin - vertex0;
    //    u = f * s.Dot(h);
    //    if (u < 0.0 || u > 1.0)
    //        continue;
    //    q = s.Cross(edge1);
    //    v = f * rayDirection.Dot(q);
    //    if (v < 0.0 || u + v > 1.0)
    //        continue;
    //    // At this stage we can compute t to find out where the intersection point is on the line.
    //    float t = f * edge2.Dot(q);
    //    if (t > EPSILON) // ray intersection
    //    {
    //        if (t < closestHitDistance) {
    //            closestHitDistance = t;
    //
    //            closestTriangleIterator = i;
    //            hit.normal = triangle.m_normalDirection;
    //        }
    //    }
    //    else // This means that there is a line intersection but not a ray intersection.
    //        continue;
    //}
    //
    //
    //if(i > -1)
    //{
    //    hit.point = rayOrigin + rayDirection * closestHitDistance;
    //    hit.distance = closestHitDistance;
    //    return true;
    //}
    //else {
    //    return false;
    //}
    return false;
}