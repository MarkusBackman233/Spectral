#pragma once
#include "MeshComponent.h"
#include "PhysicsComponent.h"
#include "LightComponent.h"
#include "ParticleComponent.h"
#include "TerrainComponent.h"
#include <memory>

class ComponentFactory
{
public:
	static std::shared_ptr<Component> CreateComponent(GameObject* gameObject, Component::ComponentType componentType, const std::shared_ptr<Component>& duplicateComponent = nullptr);
};

