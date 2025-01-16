#pragma once
#include "Scene.h"

class SceneManager
{
public:
	static SceneManager* GetInstance() {
		static SceneManager instance;
		return &instance;
	}
	SceneManager();
	Scene& GetCurrentScene();
private:
	Scene m_currentScene;
};

