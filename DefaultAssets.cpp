#include "DefaultAssets.h"
#include <vector>
#include <string>

std::pair<unsigned char*, size_t> DefaultAssets::GetDefaultAlbedoBytes()
{
	
	const unsigned int textureSize = 1024;

	unsigned char* defaultTexture = new unsigned char [textureSize * textureSize * 4];



	for (size_t x = 0; x < textureSize; x++)
	{
		for (size_t y = 0; y < textureSize * 4; y += 4)
		{
			bool odd = (x % 64 == 0) || (y % 64 == 0);

			unsigned char r = odd ? 255 : 0;
			unsigned char g = odd ? 255 : 0;
			unsigned char b = odd ? 255 : 0;
			//float texelSize = 1 / textureSize;
			//unsigned char r = x * texelSize * 255;
			//unsigned char g = y * texelSize * 255;
			//unsigned char b = 255;

			unsigned char a = 255;

			defaultTexture[x * textureSize + y]		= r;
			defaultTexture[x * textureSize + y + 1] = g;
			defaultTexture[x * textureSize + y + 2] = b;
			defaultTexture[x * textureSize + y + 3] = a;
		}
	}
	return std::make_pair(defaultTexture, textureSize * textureSize);
}
