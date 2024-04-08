#pragma once
#include "pch.h"
#include <string>
#include <memory>
#include "GameObject.h"

#include "IOManager.h"

#ifdef EDITOR
#include "Editor.h"
#endif

class Component
{
public:
	enum ComponentType
	{
		ComponentType_PhysicsComponent,
		ComponentType_MeshComponent,
		ComponentType_LightComponent,
		ComponentType_ParticleComponent,

		ComponentType_Invalid,
	};
	Component(GameObject* owner);
	virtual void Start() {};
	virtual void Reset() {};
	virtual void Update(float deltaTime) {};
	virtual void Render() {};
	
	virtual void SaveComponent(WriteObject& writeObject) {};
	virtual void LoadComponent(ReadObject& readObject) {};

	std::string GetComponentName() { return m_componentName; }

#ifdef EDITOR
	virtual void ComponentEditor() {};
#endif // EDITOR

	GameObject* GetOwner(GameObject* owner) { return m_owner; }

	template<typename T>
	inline T* Is()
	{
		return dynamic_cast<T*>(this);
	}

	ComponentType GetComponentType() const { return m_componentType; }


protected:

	std::string m_componentName;
	ComponentType m_componentType;
	GameObject* m_owner;
};

