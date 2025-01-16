#pragma once
#include "Vector3.h"

class Triangle
{
public:
	Triangle(const Math::Vector3& p1, const  Math::Vector3& p2, const  Math::Vector3& p3);
	Math::Vector3 CalculateNormal() const;
private:
	Math::Vector3 m_vertex[3];
};
