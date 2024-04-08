#include "pch.h"
#include "Component.h"
#include "IOManager.h"

Component::Component(GameObject* gameObject)
	: m_owner(gameObject)
{
	m_componentName = "Not implemented";
	m_componentType = ComponentType_Invalid;
}
