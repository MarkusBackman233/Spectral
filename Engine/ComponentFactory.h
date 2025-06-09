#pragma once
#include "pch.h"
#include "Component.h"
class ComponentFactory
{
public:
	static std::shared_ptr<Component> CreateComponent(GameObject* gameObject, Component::Type componentType, const std::shared_ptr<Component>& duplicateComponent = nullptr, bool registerComponent = true);

	static const std::unordered_map<Component::Type, std::string> ComponentTypes;
	static const std::unordered_map<std::string, Component::Type> ComponentNames;
};

