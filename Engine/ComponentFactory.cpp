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
#include "AudioSourceComponent.h"
#include "NavmeshActorComponent.h"
#include "ObjectManager.h"


// Macros to create components
#define CREATE_OR_DUPLICATE_COMPONENT(component, componentType, componentClass) \
    case componentType:                                                         \
        component = duplicateComponent ? std::make_shared<componentClass>(gameObject, static_cast<componentClass*>(duplicateComponent.get())) \
                                        : std::make_shared<componentClass>(gameObject); \
        break;

#define CREATE_COMPONENT_NO_DUPLICATE(component, componentType, componentClass) \
    case componentType:                                                         \
        component = std::make_shared<componentClass>(gameObject);               \
        break;

// Dispatch macro selector
#define COMPONENT_1(componentClass, componentEnum, str) \
    CREATE_OR_DUPLICATE_COMPONENT(component, componentEnum, componentClass)

#define COMPONENT_0(componentClass, componentEnum, str) \
    CREATE_COMPONENT_NO_DUPLICATE(component, componentEnum, componentClass)

#define COMPONENT_SELECT(canDuplicate) COMPONENT_##canDuplicate

#define COMPONENT(componentClass, componentEnum, str, canDuplicate) \
    COMPONENT_SELECT(canDuplicate)(componentClass, componentEnum, str)

// Component list
#define COMPONENT_LIST \
    COMPONENT(RigidbodyComponent,           Component::Type::Rigidbody,             "Rigidbody Component",              1) \
    COMPONENT(LightComponent,               Component::Type::Light,                 "Light Component",                  1) \
    COMPONENT(PhysicsShapeComponent,        Component::Type::PhysicsShape,          "Physics Shape Component",          1) \
    COMPONENT(MeshComponent,                Component::Type::Mesh,                  "Mesh Component",                   1) \
    COMPONENT(ParticleComponent,            Component::Type::Particle,              "Particle Component",               0) \
    COMPONENT(TerrainComponent,             Component::Type::Terrain,               "Terrain Component",                0) \
    COMPONENT(CameraComponent,              Component::Type::Camera,                "Camera Component",                 0) \
    COMPONENT(ScriptComponent,              Component::Type::Script,                "Script Component",                 1) \
    COMPONENT(CharacterControllerComponent, Component::Type::CharacterController,   "Character Controller Component",   1) \
    COMPONENT(JointComponent,               Component::Type::Joint,                 "Joint Component",                  1) \
    COMPONENT(AudioSourceComponent,         Component::Type::AudioSource,           "Audio Source Component",           1) \
    COMPONENT(NavmeshActorComponent,        Component::Type::NavmeshActor,          "Navmesh Actor Component",          1)

// Map Type to Name
const std::unordered_map<Component::Type, std::string> ComponentFactory::ComponentTypes = {
#define COMPONENT(componentClass, componentEnum, str, canDuplicate) {componentEnum, str},
    COMPONENT_LIST
#undef COMPONENT
};

// Map Name to Type
const std::unordered_map<std::string, Component::Type> ComponentFactory::ComponentNames = {
#define COMPONENT(componentClass, componentEnum, str, canDuplicate) {str, componentEnum},
    COMPONENT_LIST
#undef COMPONENT
};

// Component creation function
std::shared_ptr<Component> ComponentFactory::CreateComponent(GameObject* gameObject, Component::Type type, const std::shared_ptr<Component>& duplicateComponent)
{
    std::shared_ptr<Component> component = nullptr;

    switch (type)
    {
#define COMPONENT(componentClass, componentEnum, str, canDuplicate) \
        COMPONENT_SELECT(canDuplicate)(componentClass, componentEnum, str);
        COMPONENT_LIST
#undef COMPONENT
    }

    ObjectManager::GetInstance()->RegisterComponent(component);
    return component;
}