#include "iPhysics.h"
#include "PhysXManager.h"

namespace Physics
{
	void Simulate(float deltaTime)
	{
		PhysXManager::GetInstance()->TickSimulation(deltaTime);
	}
	RaycastHit Raycast(const Math::Vector3& origin, const Math::Vector3& direction, float maxDistance)
	{
		RaycastHit hit{};

		PxQueryFilterData filterData = PxQueryFilterData();
		filterData.data.word0 = 1;

		PxRaycastBuffer pxhit;
		if(PhysXManager::GetInstance()->GetScene()->raycast(PhysXManager::Vector3ToPxVector3(origin), PhysXManager::Vector3ToPxVector3(direction), maxDistance, pxhit, PxHitFlag::eDEFAULT, filterData))
		{
			hit.hasHit = true;
			hit.position = PhysXManager::PxVector3ToVector3(pxhit.getAnyHit(0).position);
			hit.normal = PhysXManager::PxVector3ToVector3(pxhit.getAnyHit(0).normal);
			hit.distance = pxhit.getAnyHit(0).distance;
		}
		else
		{
			hit.hasHit = false;
		}
		return hit;
	}
};