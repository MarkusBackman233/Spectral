#pragma once
#include <filesystem>

enum class ResourceType
{
	Model,
	Texture,
	Audio,
	Scene,
	Script,
	Material,
	Prefab,

	Num
};

class Resource
{
public:

	virtual ResourceType GetResourceType() = 0;

	static ResourceType StaticType();

	virtual bool Load(const std::filesystem::path& file) = 0;
	virtual void Save() {};

	void SetPath(const std::filesystem::path& path);
	std::filesystem::path GetPath();

	std::string GetFilename() const { return m_filename; }
	std::string m_filename;


	void SetUnsaved();
	void SetSaved();
	bool IsUnsaved() const;



protected:
	bool m_unsaved = false;

	std::filesystem::path m_path;
};
