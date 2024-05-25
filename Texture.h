#pragma once
#include "pch.h"
#include <string>
#include <wrl/client.h>
#include <d3d11.h>
#include "Vector2.h"
class Texture
{
public:
	Texture();

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetResourceView() { return m_textureSRV.Get(); }

	void LoadTexture(const std::string& filename);
	void LoadTexture(unsigned char* bytes, const Math::Vector2i& size);

	bool GenerateMips();


	void SetTexture(Microsoft::WRL::ComPtr<ID3D11Texture2D> texture) { m_texture = texture; }
	void SetShaderResourceView(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView) { m_textureSRV = shaderResourceView; }

private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureSRV;

};

