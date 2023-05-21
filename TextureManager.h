#pragma once
#include <iostream>
#include <SDL_image.h>
#include <string>
#include <vector>
class TextureManager
{
public:
	TextureManager();
	static TextureManager& GetInstance() {
		static TextureManager instance;
		return instance;
	}
	struct Texture
	{
		uint16_t width;
		uint16_t height;
		std::string filename;
		uint32_t* pixels;
	};

	Texture GetTexture(std::string filename);

private:
	Texture LoadTexture(std::string filename);

	std::vector<Texture> m_textures;
};

