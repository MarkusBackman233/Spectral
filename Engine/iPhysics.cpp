#include "iPhysics.h"
#include "PhysXManager.h"
#include "TerrainComponent.h"



using namespace physx;

class ActorFilterCallback : public PxQueryFilterCallback
{
public:
    PxRigidActor* targetActor;

    ActorFilterCallback(PxRigidActor* actor)
        : targetActor(actor)
    {
    }

    PxQueryHitType::Enum preFilter(
        const PxFilterData& filterData,
        const PxShape* shape,
        const PxRigidActor* actor,
        PxHitFlags& queryFlags) override
    {
        if (actor == targetActor)
            return PxQueryHitType::eBLOCK;

        return PxQueryHitType::eNONE;
    }

    PxQueryHitType::Enum postFilter(
        const PxFilterData&,
        const PxQueryHit&) override
    {
        return PxQueryHitType::eNONE;
    }
};


void Physics::Simulate(float deltaTime)
{

	PhysXManager::GetInstance()->TickSimulation(deltaTime);
}
Physics::RaycastHit Physics::Raycast(const Math::Vector3& origin, const Math::Vector3& direction, float maxDistance)
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




Physics::RaycastHit Physics::RaycastComponent(const Math::Vector3& origin, const Math::Vector3& direction, float maxDistance, Component* component)
{

    PxRigidActor* actor = nullptr;


    if (auto terrain = dynamic_cast<TerrainComponent*>(component))
    {
        actor = terrain->GetActor();
    }


    PxRaycastBuffer pxhit;

    ActorFilterCallback filter(actor);

    PxQueryFilterData filterData;
    filterData.flags |= PxQueryFlag::ePREFILTER;

    bool status = PhysXManager::GetInstance()->GetScene()->raycast(
        PhysXManager::Vector3ToPxVector3(origin),
        PhysXManager::Vector3ToPxVector3(direction),
        maxDistance,
        pxhit,
        PxHitFlag::eDEFAULT,
        filterData,
        &filter
    );

    RaycastHit hit{};

    hit.hasHit = status;
    if (status)
    {
        hit.position = PhysXManager::PxVector3ToVector3(pxhit.getAnyHit(0).position);
        hit.normal = PhysXManager::PxVector3ToVector3(pxhit.getAnyHit(0).normal);
        hit.distance = pxhit.getAnyHit(0).distance;
    }


	return hit;
}
