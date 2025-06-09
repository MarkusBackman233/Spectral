#pragma once
#include "pch.h"

#define SOL_LUAJIT 1
#define SOL_PRINT_ERRORS 0
//#define SOL_ALL_SAFETIES_ON 1
//#define SOL_EXCEPTIONS_SAFE_PROPAGATION  1
#include "src/External/Sol2/sol.hpp"
#include "Vector3.h"
#include "LoadableResource.h"
class GameObject;


class Script : public Resource
{
public:
	Script();
	~Script() {}

	bool Load(const std::filesystem::path& file) override;

	void Start(GameObject* GameObject);
	void Update(float deltaTime);

	std::vector<std::string>& GetExposedVariables();
	std::string GetFloatVariable(const std::string& variableName);

	void OnContact();

private:

	static void DrawLine(const Math::Vector3& start,const Math::Vector3& end);
	static GameObject* CreateGameObject(const std::string& name);
	static GameObject* SpawnPrefab(const std::string& name);
	static void StartGameObject(GameObject* gameObject);


	static GameObject* GetGameObjectWithName(const std::string& name);
	static void AddComponentToGameObject(GameObject* gameObject, const std::string& componentName);

	static void DuplicateGameObject(GameObject* duplicate, GameObject* source);

	void ReloadScript();

	void SetBindings();

	sol::state m_lua;

	std::vector<std::string> m_exposedVariables;
};

