#pragma once
#include "pch.h"
#include "Vector4.h"
#include "IMaterial.h"

class Texture;
class DeviceResources;
class Mesh;
struct ID3D11DeviceContext;
struct ID3D11Device;

class DefaultMaterial : public IMaterial
{
public:


	enum TextureType
	{
		BaseColor,
		Normal,
		Roughness,
		Metallic,
		AmbientOcclusion,

		NumTextures,
	};

	struct MaterialSettings
	{
		float Roughness;
		float Metallic;
		Math::Vector4 Color;
		bool BackfaceCulling;
		bool LinearFiltering;
		bool CombinedMaterialTexture;
	};
	DefaultMaterial();

	bool Load(const std::filesystem::path& file) override;

	void Render(ID3D11DeviceContext* context, const DeviceResources& deviceResources, std::shared_ptr<Mesh> mesh, InstanceManager::InstanceData instanceData) override;

	static void CreateResources(ID3D11Device* device);

	void SetTexture(int index, std::shared_ptr<Texture> texture);
	std::shared_ptr<Texture> GetTexture(int index) { return m_textures[index]; };


	MaterialSettings& GetMaterialSettings() { return m_settings; }

private:

	struct PixelConstantBuffer
	{
		Math::Vector4 data; // x = Shininess,  w = num lights
		Math::Vector4 data2; // x = hasBaseColor, y = hasNormal, z = linearFiltering, w = combined materials texture
		Math::Vector4 materialColor;
	};


	static struct MaterialGlobals
	{
		Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_pInputLayout;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_pVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_pPixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pPixelConstantBufferData;

		static_assert((sizeof(PixelConstantBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");

		PixelConstantBuffer m_pixelConstantBuffer;
	} m_materialGlobals;


	std::array<std::shared_ptr<Texture>, TextureType::NumTextures> m_textures{};

	MaterialSettings m_settings;

	bool m_combinedMaterialTexture;
};

