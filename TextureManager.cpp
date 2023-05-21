#include "TextureManager.h"

TextureManager::TextureManager()
{
	std::cout << "TextureManager initialized" << std::endl;
}

TextureManager::Texture TextureManager::GetTexture(std::string filename)
{
	for (auto texture : m_textures)
	{
		if (texture.filename == filename) {
			return texture;
		}
	}

	Texture loadedTexture = LoadTexture(filename);

	m_textures.push_back(loadedTexture);
	return loadedTexture;
}

TextureManager::Texture TextureManager::LoadTexture(std::string fileName)
{
	SDL_Surface* texture = IMG_Load(fileName.c_str());
	if (!texture)
	{
		std::cerr << "Error: Could not load texture: " << IMG_GetError() << std::endl;
		IMG_Quit();
	}
	SDL_LockSurface(texture);


	Texture loadTexture;
	loadTexture.filename = fileName;
	loadTexture.pixels = new uint32_t[texture->w * texture->h];
	loadTexture.width = texture->w;
	loadTexture.height = texture->h;

	Uint32 format = texture->format->format;
	Uint8* pixels = (Uint8*)texture->pixels;
	for (int x = 0; x < texture->w; x++)
	{
		for (int y = 0; y < texture->h; y++)
		{
			Uint8* pixel = pixels + (x * texture->pitch) + (y * texture->format->BytesPerPixel);

			Uint8 r = 0, g = 0, b = 0;
			switch (format) {
			case SDL_PIXELFORMAT_RGB24:
				r = pixel[0];
				g = pixel[1];
				b = pixel[2];
				break;
			case SDL_PIXELFORMAT_RGBA32:
				r = pixel[0];
				g = pixel[1];
				b = pixel[2];
				break;
			}
			uint32_t color = (r << 24) | (g << 16) | (b << 8) | 0xff;
			loadTexture.pixels[x * texture->w + y] = color;
		}
	}
	SDL_UnlockSurface(texture);
	return loadTexture;
}