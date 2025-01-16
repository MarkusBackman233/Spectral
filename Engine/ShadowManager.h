#pragma once
#include "pch.h"
#include <d3d11.h>
#include <minwindef.h>
#include <wtypes.h>
#include <wrl.h>
#include "Matrix.h"
#include "OrthographicCamera.h"

class Mesh;
class InstanceManager;

class ShadowManager
{
public:
	ShadowManager();

	void CreateResources(ID3D11Device* device);

	void DrawShadowDepth(ID3D11DeviceContext* context, const InstanceManager& instanceManager);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetShadowTexture() const { return m_shadowResourceView; }

	Math::Matrix GetShadowViewProjectionMatrix() const { return m_camera->GetViewProjectionMatrix(); }

	ID3D11SamplerState* GetShadowCompareSamplerState() const { return m_comparisonSamplerState.Get(); }



private:
	void SetupLightMatrix();

	std::unique_ptr<OrthographicCamera> m_camera;
	uint16_t m_ShadowMapSize;
	D3D11_VIEWPORT m_viewPort;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_shadowDepthTexture;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>   m_shadowDepthView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shadowResourceView;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>        m_pInputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>       m_pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>        m_pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>             m_pVertexConstantBufferData;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>		 m_comparisonSamplerState;
};

