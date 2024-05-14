#pragma once
#include "pch.h"

class Texture;

class TextureManager
{
public:
	TextureManager();
	std::shared_ptr<Texture> GetTexture(const std::string& filename);
	static TextureManager* GetInstance() {
		static TextureManager instance;
		return &instance;
	}
	std::string GetTextureName(std::shared_ptr<Texture> texture);
	std::map<std::string, std::shared_ptr<Texture>>& GetCachedTextures() { return m_textures; };

	void AddTexture(const std::string& textureName, std::shared_ptr<Texture> texture);
private:
	std::map<std::string, std::shared_ptr<Texture>> m_textures;
};

