#pragma once
#include "pch.h"
#include <DirectXMath.h>
#include <d3d11.h>
#include <minwindef.h>
#include <wtypes.h>
#include <wrl.h>

class Mesh;

class ShadowManager
{
public:
	static ShadowManager* GetInstance() {
		static ShadowManager instance;
		return &instance;
	}

	ShadowManager();

	void DrawShadowDepth(std::unordered_map<std::shared_ptr<Mesh>, std::vector<DirectX::XMFLOAT4X4>>& drawList);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>  GetShadowTexture() { return m_shadowResourceView; }
	bool DrawedShadows = false;

	DirectX::XMFLOAT4X4 GetShadowViewProjectionMatrix() const { return m_viewProjectionMatrix; }

	ID3D11SamplerState* GetShadowCompareSamplerState() { return m_comparisonSamplerState.Get(); }

private:
	void SetupLightMatrix();

	DirectX::XMMATRIX m_viewMatrix;
	DirectX::XMMATRIX m_projectionMatrix;
	DirectX::XMFLOAT4X4 m_viewProjectionMatrix;

	Microsoft::WRL::ComPtr<ID3D11Texture2D>        m_shadowDepthTexture;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_shadowDepthView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shadowResourceView;


	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_resourceView;

	Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_pInputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pVertexConstantBufferData;

	Microsoft::WRL::ComPtr<ID3D11Buffer>			m_pInstanceBuffer;

	Microsoft::WRL::ComPtr<ID3D11SamplerState>		m_comparisonSamplerState;

	struct VertexConstantBuffer
	{
		DirectX::XMFLOAT4X4 viewProjection;
	};

	static_assert((sizeof(VertexConstantBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");
	VertexConstantBuffer m_vertexConstantBuffer;

	unsigned int m_ShadowMapSize;
};

