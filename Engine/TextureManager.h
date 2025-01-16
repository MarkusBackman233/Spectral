#pragma once
#include "pch.h"
#include <shared_mutex>
#include "filesystem"

class Texture;

class TextureManager
{
public:
	static TextureManager* GetInstance() {
		static TextureManager instance;
		return &instance;
	}

	std::shared_ptr<Texture> GetTexture(const std::filesystem::path& file);
	std::unordered_map<std::string, std::shared_ptr<Texture>> GetCachedTextures();
	void AddTexture(std::shared_ptr<Texture> texture);
private:
	std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;

	mutable std::shared_mutex m_mutex;
};

