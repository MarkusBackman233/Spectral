#include "ComponentFactory.h"

#include "LightComponent.h"
#include "MeshComponent.h"
#include "ParticleComponent.h"
#include "TerrainComponent.h"
#include "RigidbodyComponent.h"
#include "PhysicsShapeComponent.h"
#include "CameraComponent.h"
#include "ScriptComponent.h"
#include "CharacterControllerComponent.h"
#include "JointComponent.h"

// Use if component has a duplicate constructor
#define CREATE_OR_DUPLICATE_COMPONENT(componentType,componentClass) \
case componentType:                                                 \
     return duplicateComponent ? std::make_shared<componentClass>(gameObject, static_cast<componentClass*>(duplicateComponent.get())) : std::make_shared<componentClass>(gameObject)

// Use if component does not have a duplicate constructor
#define CREATE_COMPONENT_NO_DUPLICATE(componentType,componentClass) \
case componentType:                                    \
     return std::make_shared<componentClass>(gameObject)

const std::unordered_map<Component::Type, std::string> ComponentFactory::ComponentTypes =
{
    {Component::Type::Mesh,"Mesh Component"},
    {Component::Type::Light,"Light Component"},
    {Component::Type::Particle,"Particle Component"},
    {Component::Type::Terrain,"Terrain Component"},
    {Component::Type::Rigidbody,"Rigidbody Component"},
    {Component::Type::PhysicsShape,"Physics Shape Component"},
    {Component::Type::Joint,"Joint Component"},
    {Component::Type::Camera,"Camera Component"},
    {Component::Type::Script,"Script Component"},
    {Component::Type::CharacterController,"Character Controller Component"},
};

const std::unordered_map<std::string, Component::Type> ComponentFactory::ComponentNames =
{
    {"Mesh Component",Component::Type::Mesh},
    {"Light Component",Component::Type::Light},
    {"Particle Component",Component::Type::Particle},
    {"Terrain Component",Component::Type::Terrain},
    {"Rigidbody Component",Component::Type::Rigidbody},
    {"Physics Shape Component",Component::Type::PhysicsShape},
    {"Joint Component",Component::Type::Joint},
    {"Camera Component",Component::Type::Camera},
    {"Script Component",Component::Type::Script},
    {"Character Controller Component",Component::Type::CharacterController},
};


std::shared_ptr<Component> ComponentFactory::CreateComponent(GameObject* gameObject, Component::Type componentType, const std::shared_ptr<Component>& duplicateComponent)
{
    switch (componentType)
    {
    CREATE_OR_DUPLICATE_COMPONENT(Component::Type::Rigidbody, RigidbodyComponent);
    CREATE_OR_DUPLICATE_COMPONENT(Component::Type::Light, LightComponent);
    CREATE_OR_DUPLICATE_COMPONENT(Component::Type::PhysicsShape, PhysicsShapeComponent);
    CREATE_OR_DUPLICATE_COMPONENT(Component::Type::Mesh, MeshComponent);
    CREATE_COMPONENT_NO_DUPLICATE(Component::Type::Particle, ParticleComponent);
    CREATE_COMPONENT_NO_DUPLICATE(Component::Type::Terrain, TerrainComponent);
    CREATE_COMPONENT_NO_DUPLICATE(Component::Type::Camera, CameraComponent);
    CREATE_OR_DUPLICATE_COMPONENT(Component::Type::Script, ScriptComponent);
    CREATE_OR_DUPLICATE_COMPONENT(Component::Type::CharacterController, CharacterControllerComponent);
    CREATE_OR_DUPLICATE_COMPONENT(Component::Type::Joint, JointComponent);
    }


    return nullptr;
}