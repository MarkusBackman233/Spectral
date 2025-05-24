#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "Vector4.h"
#include <memory>
#include <vector>

struct Light;

class Scene
{
public:
	struct LightingSettings
	{
		Math::Vector4 AmbientLight;
		Math::Vector4 FogColor;

		float SSAORadius;
		float SSAOIntensity;
		float SSAOBias;

		float ShadowCameraSize;
		float NearDepth;
		float FarDepth;
		float CameraDistance;
		float gamma;
	};

	Scene();

	void LoadSceneSkybox();

	Light* GetSun();
	LightingSettings& GetLightingSettings() { return m_lightingSettings; }
	std::vector<std::shared_ptr<Light>>& GetLights() { return m_lights; }


	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetIradianceResourceView() { return m_radianceResourceView.Get(); }
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSpecularResourceView() { return m_specularResourceView.Get(); }
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSkyboxResourceView() { return m_skyboxResourceView.Get(); }
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSpecularIntegrationResourceView() { return m_specularIntegrationResourceView.Get(); }

private:
	std::vector<std::shared_ptr<Light>> m_lights;

	LightingSettings m_lightingSettings;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_radianceResourceView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_specularResourceView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_specularIntegrationResourceView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_skyboxResourceView;
};

