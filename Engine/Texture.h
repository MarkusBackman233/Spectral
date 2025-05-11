#pragma once
#include "pch.h"
#include <wrl/client.h>
#include <d3d11.h>
#include "Vector2.h"
#include "LoadableResource.h"

class Texture : public Resource
{
public:
	Texture() {};
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetResourceView() { return m_textureSRV; }

	bool LoadTexture(unsigned char* bytes, const Math::Vector2i& size);
	bool Load(const std::filesystem::path& file) override;
	bool LoadFromResource(unsigned char* bytes, size_t size);
private:
	void GenerateMips(ID3D11Texture2D* texture);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureSRV;
};

