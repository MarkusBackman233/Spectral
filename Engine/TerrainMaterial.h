#pragma once
#include "pch.h"
#include "Vector4.h"
#include "Vector3.h"
#include "IMaterial.h"
#include <bitset>
class Texture;
class DeviceResources;
class Mesh;
class DefaultMaterial;
struct ID3D11DeviceContext;
struct ID3D11Device;

class TerrainMaterial : public IMaterial
{
public:

	TerrainMaterial();

	bool Load(const std::filesystem::path& file) override;

	void Render(ID3D11DeviceContext* context, const DeviceResources& deviceResources, std::shared_ptr<Mesh> mesh, InstanceManager::InstanceData instanceData);

	static void CreateResources(ID3D11Device* device);

	static Math::Vector3 g_mouseRayHit;
	static float g_brushSize;

	std::array<std::shared_ptr<DefaultMaterial>, 4> m_materials{};

private:

	enum MaterialDataBitmaskIndex
	{
		Albedo0,
		Albedo1,
		Albedo2,
		Albedo3,
		Normal0,
		Normal1,
		Normal2,
		Normal3,
		Roughness0,
		Roughness1,
		Roughness2,
		Roughness3,
	};


	struct PixelConstantBuffer
	{
		Math::Vector3 RaycastHitPos;
		float BrushSize;
		std::bitset<32> MaterialData;
		float Unused1;
		float Unused2;
		float Unused3;
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

};

