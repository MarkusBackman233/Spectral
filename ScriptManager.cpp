#include "ScriptManager.h"
#include "Script.h"
#include "mutex"
#include "Logger.h"
ScriptManager::ScriptManager()
{
}

void ScriptManager::CreateScript(const std::string& filename)
{
	auto script = std::make_shared<Script>(filename, true);
	std::shared_lock lock(m_mutex);
	m_scripts.emplace(filename, script);
}

std::shared_ptr<Script> ScriptManager::GetScript(const std::string& filename)
{
	{
		std::shared_lock lock(m_mutex); // Shared lock for reading
		auto it = m_scripts.find(filename);
		if (it != m_scripts.end())
		{
			return it->second;
		}
	}

	auto script = std::make_shared<Script>(filename, false);
	std::unique_lock lock(m_mutex);
	m_scripts.emplace(filename, script);

	Logger::Info("Loading Script: " + filename);
	return script;
}
