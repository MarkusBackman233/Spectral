#pragma once
#include "Vector3.h"


class Component;

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

	// Raycast only against specific component
	RaycastHit RaycastComponent(const Math::Vector3& origin,const Math::Vector3& direction, float maxDistance, Component* component);
};
