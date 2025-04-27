#pragma once
#include "pch.h"
#include "rapidjson/document.h"
#include "Json.h"
class GameObject;
class Component
{
public:
	enum class Type : uint8_t
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
		AudioSource,

		Num,

		Invalid,
	};

	Component(GameObject* owner);
	virtual Component::Type GetComponentType() { return Component::Type::Invalid; }


	virtual void Start() {};
	virtual void Reset() {};
	virtual void Update(float deltaTime) {};
	virtual void Render() {};
	
	virtual Json::Object SaveComponent() { return Json::Object(); }
	virtual void LoadComponent(const rapidjson::Value& object) {}

#ifdef EDITOR
	virtual void ComponentEditor() {};
	virtual void DisplayComponentIcon() {};
#endif // EDITOR

	GameObject* GetOwner() { return m_owner; }

	virtual std::string GetComponentName();

	template<typename T>
	inline T* Is()
	{
		return dynamic_cast<T*>(this);
	}

protected:

	GameObject* m_owner;
};

