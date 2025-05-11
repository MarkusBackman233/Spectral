#pragma once

#include <filesystem>

class Resource
{
public:
	virtual bool Load(const std::filesystem::path& file) = 0;

	std::string GetFilename() const { return m_filename; }

	std::string m_filename;
};
