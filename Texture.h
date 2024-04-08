#pragma once
#include "pch.h"
#include <string>
#include <wrl/client.h>
#include <d3d11.h>
class Texture
{
public:
	Texture();

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetResourceView() { return m_textureSRV.Get(); }

	void LoadTexture(const std::string& filename);

	bool GenerateMips();

private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureSRV;

};

