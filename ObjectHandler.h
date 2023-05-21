#pragma once
#include "GameObject.h"
class ObjectHandler
{
public:

	void Instantiate(GameObject &gameObject);



	std::vector<GameObject*> GetGameObjects() { return m_gameObjects; }

private:
	std::vector<GameObject*> m_gameObjects;
};

