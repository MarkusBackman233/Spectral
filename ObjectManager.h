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
	GameObject* CreateObject(const std::string& name);
	void Destroy(GameObject* gameObject);

	std::vector<GameObject*> GetGameObjects() { return m_gameObjects; }

private:
	std::vector<GameObject*> m_gameObjects;
};
