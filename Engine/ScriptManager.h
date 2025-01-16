#pragma once
#include "pch.h"
#include <shared_mutex>
class Script;

class ScriptManager
{
public:
	static ScriptManager* GetInstance() {
		static ScriptManager instance;
		return &instance;
	}
	ScriptManager();

	void CreateScript(const std::string& filename);

	std::shared_ptr<Script> GetScript(const std::string& filename);

	std::unordered_map<std::string, std::shared_ptr<Script>>& GetScripts() { return m_scripts; };

private:
	std::unordered_map<std::string, std::shared_ptr<Script>> m_scripts;
	mutable std::shared_mutex m_mutex;
};

