#pragma once
#include "pch.h"
#include "GameObject.h"
#include <unordered_set>
#include "Component.h"
#include <shared_mutex>



class ObjectManager
{
public:
	static ObjectManager* GetInstance() {
		static ObjectManager instance;
		return &instance;
	}
	ObjectManager();
	void Deinitialize();
	GameObject* CreateObject(const std::string& name, bool destroyObjectOnReset = false);
	void Destroy(GameObject* gameObject);

	const std::unordered_set<std::unique_ptr<GameObject>>& GetGameObjects() { return m_gameObjects; }

	void RegisterComponent(std::shared_ptr<Component> component);
	void UnregisterComponent(std::shared_ptr<Component> component);

	std::vector<std::weak_ptr<Component>> GetComponentsOfType(Component::Type type);

	void Start();
	void Reset();
	void Update(float deltaTime);
	void Render();

	GameObject* GetMainCameraGameObject();



private:
	std::unordered_set<std::unique_ptr<GameObject>> m_gameObjects;
	std::unordered_map<Component::Type, std::vector<std::weak_ptr<Component>>> m_components;

	mutable std::shared_mutex m_getObjectsMutex;

	size_t m_currentlyReserved;
};
