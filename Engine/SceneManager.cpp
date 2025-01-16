#include "SceneManager.h"


SceneManager::SceneManager()
{

}

Scene& SceneManager::GetCurrentScene()
{
	return m_currentScene;
}
