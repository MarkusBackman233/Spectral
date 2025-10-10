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


	DeferredPipeline();

	void CreateResources(ID3D11Device* device, const Math::Vector2& windowSize);
	void ReleaseResources();

	void RenderGBuffer(ID3D11DeviceContext* context, const DeviceResources& deviceResources, const InstanceManager& instanceManager, const ShadowManager& shadowmanager, Math::Vector2 viewportSize);
	
	ID3D11ShaderResourceView* GetSRV() const;

private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_gBufferTexture;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_gBufferRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_gBufferSRV;

	Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pVertexConstantBufferData;

	struct VertexConstantBuffer
	{
		Math::Matrix viewProjection;
		Math::Vector4 cameraPos;
	};

	static_assert((sizeof(VertexConstantBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");

	VertexConstantBuffer m_vertexConstantBuffer;

};

