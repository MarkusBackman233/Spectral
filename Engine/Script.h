#pragma once
#include "pch.h"

#define SOL_LUAJIT 1
#define SOL_ALL_SAFETIES_ON 1
#define SOL_EXCEPTIONS_SAFE_PROPAGATION  1
#include "src/External/Sol2/sol.hpp"
#include "Vector3.h"

class GameObject;


class Script
{
public:
	Script(const std::string& filename, bool newScript = false);
	~Script() {}

	std::string GetFilename() const { return m_filename; }

	void Start(GameObject* GameObject);
	void Update(float deltaTime);

	std::vector<std::string>& GetExposedVariables();
	std::string GetFloatVariable(const std::string& variableName);

private:


	static void DrawLine(const Math::Vector3& start,const Math::Vector3& end);
	static GameObject* CreateGameObject(const std::string& name);
	static GameObject* GetGameObjectWithName(const std::string& name);
	static void AddComponentToGameObject(GameObject* gameObject, const std::string& componentName);

	void ReloadScript();

	void SetBindings();

	std::string m_filename;
	sol::state m_lua;

	std::vector<std::string> m_exposedVariables;
};

