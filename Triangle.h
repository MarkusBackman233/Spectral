#pragma once
#include "pch.h"
#include "Vector3.h"
#include "Vector2.h"
#include <cmath>
#include <stdint.h>

class Triangle
{
public:
	Triangle(void) {}
	Triangle(const Math::Vector3& p1, const  Math::Vector3& p2, const  Math::Vector3& p3);

	Math::Vector3 CalculateNormal() const;
	int ClipAgainstPlane(const Math::Vector3& planePosition, const Math::Vector3& planeNormal, Triangle& out_tri1, Triangle& out_tri2) const;

	Math::Vector3 m_normalDirection;
	Math::Vector3 m_vertex[3];
	Math::Vector2 m_UV[3];

private:
};
