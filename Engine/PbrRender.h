#pragma once
#include "PostProcessing.h"
#include "Vector4.h"
#include "Matrix.h"

class DeferredPipeline;
class SkyboxManager;
class ShadowManager;
class SSAO;
struct Light;
struct ID3D11Device;

class PbrRender :
    public PostProcessing
{
public:
    PbrRender();

	void CreateResources(ID3D11Device* device);

    void Process(
		ID3D11DeviceContext* context, 
		const DeviceResources& deviceResources, 
		const DeferredPipeline& deferredPipeline, 
		const SkyboxManager& skyboxManager, 
		const ShadowManager& shadowManager
	);

	void RenderLight(const Light* light);

private:

	struct LightShaderData
	{
		Math::Vector4 position;
		Math::Vector4 direction;
		Math::Vector4 color;
		Math::Vector4 additionalData; // x = lightType, y = attenuation, z = enabled
	};
	struct PixelConstantBuffer
	{
		Math::Matrix viewProjection;
		Math::Matrix lightMatrix;
		Math::Matrix Projection;
		Math::Matrix viewProjectionInverse;
		Math::Matrix ProjectionInverse;

		Math::Vector4 ambientLighting;
		Math::Vector4 fogColor;
		Math::Vector3 cameraPosition;
		uint32_t numLights;
		Math::Vector4 gamma;

		LightShaderData lights[50];
	};

	struct SSAOPixelConstantBuffer
	{
		Math::Vector4 settings;
		Math::Vector4 kernelPosition[64];
	} m_SSAOpixelConstantBuffer;

	Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pSSAOPixelConstantBufferData;
	Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pPixelConstantBufferData;
	PixelConstantBuffer m_pixelConstantBuffer;


	std::vector<const Light*> m_lightsToRender;

};

