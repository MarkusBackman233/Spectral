#include "CharacterControllerComponent.h"
#include "src/IMGUI/imgui.h"
#include "GameObject.h"
#include "iPhysics.h"
CharacterControllerComponent::CharacterControllerComponent(GameObject* owner)
	: Component(owner)
	, m_controller(nullptr)
    , m_isOnGround(false)
{

}

CharacterControllerComponent::CharacterControllerComponent(GameObject* owner, CharacterControllerComponent* characterControllerComponent)
	: Component(owner)
	, m_controller(nullptr)
    , m_isOnGround(false)
{
}

CharacterControllerComponent::~CharacterControllerComponent()
{

}

void CharacterControllerComponent::Start()
{
    auto scale = m_owner->GetWorldMatrix().GetScale();

    PxCapsuleControllerDesc desc;
    desc.height = scale.y;
    desc.radius = std::max(scale.x, scale.z);
    desc.position.x = m_owner->GetWorldMatrix().GetPosition().x;
    desc.position.y = m_owner->GetWorldMatrix().GetPosition().y;
    desc.position.z = m_owner->GetWorldMatrix().GetPosition().z;
    desc.material = PhysXManager::GetInstance()->GetDefaultMaterial();
    desc.stepOffset = 0.1f;
    desc.contactOffset = 0.08f;
    desc.slopeLimit = cosf(PxPi / 3.0f); // 60 degrees slope limit

    m_controller = PhysXManager::GetInstance()->GetControllerManager()->createController(desc);




    auto actor = m_controller->getActor();

    PxU32 numShapes = actor->getNbShapes();
    PxShape** shapes = new PxShape * [numShapes];
    actor->getShapes(shapes, numShapes);
    for (PxU32 i = 0; i < numShapes; i++)
    {
        PxFilterData filter;
        filter.word0 = 2;
        shapes[i]->setQueryFilterData(filter);
    }
    delete[] shapes;
}

void CharacterControllerComponent::Reset()
{
    m_controller->release();
}

void CharacterControllerComponent::Update(float deltaTime)
{
    PxControllerFilters filters;
    PxControllerCollisionFlags collisionFlags = m_controller->move(PhysXManager::Vector3ToPxVector3(m_moveDisplacement), 0.001f, deltaTime, filters);
    m_isOnGround = collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN;

    m_owner->SetPosition(PhysXManager::PxVector3ToVector3(m_controller->getPosition()));
    //auto hit = Physics::Raycast(m_owner->GetWorldMatrix().GetPosition(), Math::Vector3(0,-1,0), m_owner->GetWorldMatrix().GetScale().y + 0.15f);
    //m_isOnGround = hit.hasHit;
}

Json::Object CharacterControllerComponent::SaveComponent()
{
    return Json::Object();
	//object.AddMember("Physics Type", GetPhysicsType(), allocator);
}

void CharacterControllerComponent::LoadComponent(const rapidjson::Value& object)
{
	//SetPhysicsType(static_cast<PhysXManager::PhysicsType>(object["Physics Type"].GetInt()));
}

#ifdef EDITOR
void CharacterControllerComponent::ComponentEditor()
{

}
#endif // EDITOR

bool CharacterControllerComponent::IsOnGround() const
{
    return m_isOnGround;
}

void CharacterControllerComponent::Move(const Math::Vector3& displacement)
{
    m_moveDisplacement = displacement;
}
