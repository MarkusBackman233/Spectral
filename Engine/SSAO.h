#pragma once
#include "PostProcessing.h"
#include <d3d11.h>
#include <wrl.h>
#include "Matrix.h"
#include "Vector4.h"
#include "Vector2.h"

class DeferredPipeline;

class SSAO :
	public PostProcessing
{

public:
	SSAO();

	void CreateResources(ID3D11Device* device, const Math::Vector2& windowSize);
	void Process(ID3D11DeviceContext* context, const DeviceResources& deviceResources, const DeferredPipeline& deferredPipeline);
	void ReleaseResources();

	ID3D11ShaderResourceView* GetSRV() const;

private:
	struct PixelConstantBuffer
	{

		Math::Matrix viewProjectionMatrix;
		Math::Vector4 settings;
		Math::Vector4 kernelPosition[64];
	} m_pixelConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pPixelConstantBufferData;


	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_SSAOTexture;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_SSAORTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SSAOSRV;
};

