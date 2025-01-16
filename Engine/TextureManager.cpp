#include "TextureManager.h"
#include "Texture.h"
#include <shared_mutex>

std::shared_ptr<Texture> TextureManager::GetTexture(const std::filesystem::path& file)
{
	std::shared_ptr<Texture> texture(nullptr);
	std::string filename = file.filename().string();

	{
		std::shared_lock lock(m_mutex); // Shared lock for reading

		auto textureIt = m_textures.find(filename);
		if (textureIt != m_textures.end())
		{
			return textureIt->second;
		}
	}
	{
		texture = std::make_shared<Texture>();
		std::unique_lock lock(m_mutex);
		m_textures.try_emplace(filename, texture);
	}

	if (!texture->LoadTexture(file))
	{
		std::unique_lock lock(m_mutex); // Exclusive lock for writing

		m_textures.erase(filename);
		return nullptr;
	}
	texture->SetFilename(filename);
	return texture;
}

std::unordered_map<std::string, std::shared_ptr<Texture>> TextureManager::GetCachedTextures()
{
	std::shared_lock lock(m_mutex); // Shared lock for reading

	return m_textures;
}

void TextureManager::AddTexture(std::shared_ptr<Texture> texture)
{
	std::unique_lock lock(m_mutex); // Exclusive lock for writing

	m_textures.try_emplace(texture->GetFilename(), texture);
}
