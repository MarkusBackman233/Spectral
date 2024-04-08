#include "ComponentFactory.h"

std::shared_ptr<Component> ComponentFactory::CreateComponent(GameObject* gameObject, Component::ComponentType componentType, const std::shared_ptr<Component>& duplicateComponent)
{
    switch (componentType)
    {
    case Component::ComponentType_LightComponent:
        return  duplicateComponent ? std::make_shared<LightComponent>(gameObject, static_cast<LightComponent*>(duplicateComponent.get())) : std::make_shared<LightComponent>(gameObject);
    case Component::ComponentType_PhysicsComponent:
        return duplicateComponent ? std::make_shared<PhysicsComponent>(gameObject, static_cast<PhysicsComponent*>(duplicateComponent.get())) : std::make_shared<PhysicsComponent>(gameObject);
    case Component::ComponentType_MeshComponent:
        return duplicateComponent ? std::make_shared<MeshComponent>(gameObject, static_cast<MeshComponent*>(duplicateComponent.get())) : std::make_shared<MeshComponent>(gameObject);
    case Component::ComponentType_ParticleComponent:
        return std::make_shared<ParticleComponent>(gameObject);
    }
    return nullptr;
}