#include "TextureManager.h"
#include "Logger.h"
#include <algorithm>
#include "Texture.h"
#include "StringUtils.h"
#include "mutex"

TextureManager::TextureManager() 
	: m_textures{}
{
	LogMessage("TextureManager initialized");
}

std::shared_ptr<Texture> TextureManager::GetTexture(const std::string& filename)
{
	static std::mutex mutex;
	mutex.lock();
	
	auto texture = m_textures.find(filename);
	if (texture != m_textures.end())
	{
		mutex.unlock();
		return m_textures[filename];
	}
	m_textures[filename] = std::make_shared<Texture>();
	mutex.unlock();

	m_textures[filename]->LoadTexture(filename);
	return m_textures[filename];
}

std::string TextureManager::GetTextureName(std::shared_ptr<Texture> texture)
{
	for (const auto& [filename, cachedTexture] : m_textures)
	{
		if (cachedTexture == texture)
		{
			return filename;
		}
	}

	return "";
}

void TextureManager::AddTexture(const std::string& textureName, std::shared_ptr<Texture> texture)
{
	m_textures[textureName] = texture;
}
