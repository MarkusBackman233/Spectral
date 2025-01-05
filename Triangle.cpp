#include "Triangle.h"


Triangle::Triangle(const Math::Vector3& p1, const  Math::Vector3& p2, const  Math::Vector3& p3)
{
	m_vertex[0] = p1;
	m_vertex[1] = p2;
	m_vertex[2] = p3;
}


Math::Vector3 Triangle::CalculateNormal() const
{
    const Math::Vector3 U = m_vertex[1] - m_vertex[0];
    const Math::Vector3 V = m_vertex[2] - m_vertex[0];

    Math::Vector3 normal(
        U.y * V.z - U.z * V.y,
        U.z * V.x - U.x * V.z,
        U.x * V.y - U.y * V.x
    );

    return -normal.GetNormal();
}
