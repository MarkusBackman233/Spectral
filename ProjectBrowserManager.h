#pragma once
#ifdef EDITOR
#include "pch.h"
#include <filesystem>

class ProjectBrowserManager
{
public:
	ProjectBrowserManager();

	bool HasLoadedProject();

private:

	struct PreviousProject
	{
		PreviousProject(const std::filesystem::path& path, const std::string& name)
		{
			Path = path;
			Name = name;
		}
		std::filesystem::path Path;
		std::string Name;
	};
	bool Update();


	std::filesystem::path SelectFromFileExplorer();

	void AddToPreviousProject(const std::filesystem::path& path) const;
	void ReadPreviousProjects();
	void RemoveProjectFromOldProjects(const std::filesystem::path& path);

	void CreateProject(const std::filesystem::path& path, const std::string& name);
	void SelectProject(const std::filesystem::path& path, const std::string& name);
	void LoadProject();
	bool DoesProjectFileExist(const std::filesystem::path& path);
	std::string ReadProjectNameFromIni(const std::filesystem::path& path);

	std::deque<PreviousProject> m_previousProjects;
	std::filesystem::path m_previousProjectsPath;
	bool m_hasLoadedProject;
};
#endif