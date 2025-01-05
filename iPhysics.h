#pragma once
#include "Vector3.h"

namespace Physics
{
	struct RaycastHit
	{
		bool hasHit;
		Math::Vector3 position;
		Math::Vector3 normal;
		float distance;
	};

	void Simulate(float deltaTime);
	RaycastHit Raycast(const Math::Vector3& origin,const Math::Vector3& direction, float maxDistance);
};
