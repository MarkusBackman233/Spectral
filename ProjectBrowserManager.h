#pragma once
#include "pch.h"
#include <filesystem>

class ProjectBrowserManager
{
public:
	static ProjectBrowserManager* GetInstance() {
		static ProjectBrowserManager instance;
		return &instance;
	}

	ProjectBrowserManager();

	bool Update();
private:

	struct PreviousProject
	{
		PreviousProject(const std::string& path, const std::string& name)
		{
			Path = path;
			Name = name;
		}

		std::string Path;
		std::string Name;
	};

	std::filesystem::path SelectFromFileExplorer();

	void AddToPreviousProject(const std::filesystem::path& path) const;
	void ReadPreviousProjects();
	void RemoveProjectFromOldProjects(const std::filesystem::path& path);

	void CreateProject(const std::filesystem::path& path, const std::string& name);
	void LoadProject();
	bool DoesProjectFileExist(const std::filesystem::path& path);

	std::string ReadProjectNameFromIni(const std::filesystem::path& path);

	std::deque<PreviousProject> m_previousProjects;
	std::string m_previousProjectsFilename;
};
