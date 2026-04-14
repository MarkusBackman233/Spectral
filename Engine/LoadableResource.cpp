#include "LoadableResource.h"

void Resource::SetPath(const std::filesystem::path& path)
{
	m_path = path;
}

std::filesystem::path Resource::GetPath()
{
	return m_path;
}

void Resource::SetUnsaved()
{
	m_unsaved = true;
}

void Resource::SetSaved()
{
	m_unsaved = false;
}

bool Resource::IsUnsaved() const
{
	return m_unsaved;
}
