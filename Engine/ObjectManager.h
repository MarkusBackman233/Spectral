#pragma once
#include "pch.h"
#include "GameObject.h"
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

	std::vector<std::unique_ptr<GameObject>>& GetGameObjects() { return m_gameObjects; }

	void Start();
	void Reset();
	void Update(float deltaTime);
	void Render();

	GameObject* GetMainCameraGameObject();

private:
	std::vector<std::unique_ptr<GameObject>> m_gameObjects;
};
