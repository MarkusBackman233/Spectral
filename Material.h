#pragma once
#include "pch.h"
#include <memory>
#include <d3d11.h>
#include <DirectXMath.h>
#include "wrl.h"
#include "Vector3.h"
#define ALBEDO 0
#define NORMAL 1
#define ROUGHNESS 2
#define METALLIC 3
#define AO 4

class Texture;

class Material
{
public:
	struct MaterialSettings
	{
		float Roughness;
		float Metallic;
		float Ao;
		Math::Vector3 Color;
	};

	Material();

	void SetupShader();

	void PrepareMaterialGlobals();


	virtual void PreparePerMaterial();

	void SetTexture(int index, std::shared_ptr<Texture> texture);
	std::shared_ptr<Texture> GetTexture(int index) { return m_textures[index]; };
	void SetName(const std::string& name) { m_name = name; }
	std::string& GetName() { return m_name; }

	MaterialSettings& GetMaterialSettings() { return m_materialSettings; }

private:
	static Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_pInputLayout;
	static Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_pVertexShader;
	static Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_pPixelShader;
	static Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pVertexConstantBufferData;
	static Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pPixelConstantBufferData;

	struct VertexConstantBuffer 
	{
		DirectX::XMFLOAT4X4 viewProjection;
		DirectX::XMFLOAT4X4 directionLightViewProjection;
		DirectX::XMFLOAT4 cameraPosition;
	};

	struct PixelConstantBuffer 
	{
		DirectX::XMFLOAT4 ambientLighting;
		DirectX::XMFLOAT4 fogColor;
		DirectX::XMFLOAT4 data; // x = Shininess
	};

	static bool HasSetupShader;

	static_assert((sizeof(VertexConstantBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");
	static_assert((sizeof(PixelConstantBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");
	static VertexConstantBuffer m_vertexConstantBuffer;
	PixelConstantBuffer m_pixelConstantBuffer;

	std::string m_name;
	std::shared_ptr<Texture> m_textures[5];

	MaterialSettings m_materialSettings;

};

