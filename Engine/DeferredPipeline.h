#pragma once

#include <d3d11.h>
#include <wrl.h>
#include "matrix.h"
#include "Vector2.h"

class Mesh;
class ShadowManager;
class InstanceManager;
class DeviceResources;

class DeferredPipeline
{
public:
	enum GBufferTexture : uint8_t
	{
		Albedo,
		Normal,
		WorldPosition,
		LightPosition,

		NumTextures
	};

	DeferredPipeline();

	void CreateResources(ID3D11Device* device, const Math::Vector2& windowSize);
	void ReleaseResources();

	void RenderGBuffer(ID3D11DeviceContext* context, const DeviceResources& deviceResources, const InstanceManager& instanceManager, const ShadowManager& shadowmanager);
	
	ID3D11ShaderResourceView* GetSRV(GBufferTexture texture) const;

private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_gBufferTextures[GBufferTexture::NumTextures];
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_gBufferRTVs[GBufferTexture::NumTextures];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_gBufferSRVs[GBufferTexture::NumTextures];

	Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_pInputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pVertexConstantBufferData;
	Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pPixelConstantBufferData;

	struct PixelConstantBuffer
	{
		Math::Vector4 data; // x = Shininess,  w = num lights
		Math::Vector4 data2; // x = hasBaseColor, y = hasNormal, z = linearFiltering
		Math::Vector4 materialColor;
	};

	struct VertexConstantBuffer
	{
		Math::Matrix viewProjection;
		Math::Matrix lightMatrix;
		Math::Vector4 cameraPos;
	};

	static_assert((sizeof(VertexConstantBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");
	static_assert((sizeof(PixelConstantBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");

	VertexConstantBuffer m_vertexConstantBuffer;
	PixelConstantBuffer m_pixelConstantBuffer;

};

