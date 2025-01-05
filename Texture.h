#pragma once
#include "pch.h"
#include <wrl/client.h>
#include <d3d11.h>
#include "Vector2.h"

class Texture
{
public:
	Texture() {};
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetResourceView() { return m_textureSRV; }

	bool LoadTexture(unsigned char* bytes, const Math::Vector2i& size);
	bool LoadTexture(const std::filesystem::path& file);

	std::string GetFilename();
	void SetFilename(const std::string& filename);
private:
	void GenerateMips(ID3D11Texture2D* texture);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureSRV;
	std::string m_filename;
};

