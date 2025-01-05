#include "pch.h"
#include "Component.h"
#include "IOManager.h"
#include "ComponentFactory.h"
#include "Logger.h"
Component::Component(GameObject* gameObject)
	: m_owner(gameObject)
{

}

std::string Component::GetComponentName()
{
	auto it = ComponentFactory::ComponentTypes.find(GetComponentType());
	if (it != ComponentFactory::ComponentTypes.end())
	{
		return it->second;
	}
	
	Logger::Error("Could not find component name!");
	return "Not Found!";
}
