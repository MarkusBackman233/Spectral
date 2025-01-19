#pragma once
#include "pch.h"
#include "rapidjson/document.h"
class GameObject;
class Component
{
public:
	enum class Type
	{
		Rigidbody,
		PhysicsShape,
		Joint,
		Mesh,
		Light,
		Particle,
		Terrain,
		Camera,
		Script,
		CharacterController,

		Num,

		Invalid,
	};

	Component(GameObject* owner);
	virtual Component::Type GetComponentType() { return Component::Type::Invalid; }


	virtual void Start() {};
	virtual void Reset() {};
	virtual void Update(float deltaTime) {};
	virtual void Render() {};
	
	virtual void SaveComponent(rapidjson::Value& object, rapidjson::Document::AllocatorType& allocator) {};
	virtual void LoadComponent(const rapidjson::Value& object) {};

#ifdef EDITOR
	virtual void ComponentEditor() {};
	virtual void DisplayComponentIcon() {};
#endif // EDITOR

	GameObject* GetOwner(GameObject* owner) { return m_owner; }

	virtual std::string GetComponentName();

	template<typename T>
	inline T* Is()
	{
		return dynamic_cast<T*>(this);
	}

protected:

	GameObject* m_owner;
};

