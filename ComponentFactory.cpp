#include "ComponentFactory.h"

#define CREATE_OR_DUPLICATE_COMPONENT(component) \
     duplicateComponent ? std::make_shared<component>(gameObject, static_cast<component*>(duplicateComponent.get())) : std::make_shared<component>(gameObject)

std::shared_ptr<Component> ComponentFactory::CreateComponent(GameObject* gameObject, Component::ComponentType componentType, const std::shared_ptr<Component>& duplicateComponent)
{
    switch (componentType)
    {
    case Component::ComponentType_LightComponent:
        return CREATE_OR_DUPLICATE_COMPONENT(LightComponent);
    case Component::ComponentType_PhysicsComponent:
        return CREATE_OR_DUPLICATE_COMPONENT(PhysicsComponent);
    case Component::ComponentType_MeshComponent:
        return CREATE_OR_DUPLICATE_COMPONENT(MeshComponent);
    case Component::ComponentType_ParticleComponent:
        return std::make_shared<ParticleComponent>(gameObject);    
    case Component::ComponentType_TerrainComponent:
        return std::make_shared<TerrainComponent>(gameObject);
    }
    return nullptr;
}