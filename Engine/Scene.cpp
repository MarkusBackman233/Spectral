#include "Scene.h"
#include "ObjectManager.h"

#include "iRender.h"
#include "IOManager.h"


#include "src/External/stb_image.h"
#include "src/External/DDSTextureLoader/DDSTextureLoader11.h"

Scene::Scene()
{
	m_lightingSettings.AmbientLight.x = 0.01f;
	m_lightingSettings.AmbientLight.y = 0.01f;
	m_lightingSettings.AmbientLight.z = 0.01f;
	m_lightingSettings.AmbientLight.w = 1.0f;


	m_lightingSettings.FogColor.x = 0.3f;
	m_lightingSettings.FogColor.y = 0.3f;
	m_lightingSettings.FogColor.z = 0.3f;
	m_lightingSettings.FogColor.w = 0.1f;

	m_lightingSettings.ShadowCameraSize = 600.0f;
	m_lightingSettings.NearDepth = 18.0f;
	m_lightingSettings.FarDepth = 524.0f;
	m_lightingSettings.CameraDistance = 335.0f;
	m_lightingSettings.gamma = 2.2f;


	m_lightingSettings.SSAOIntensity = 2.5f;
	m_lightingSettings.SSAORadius = 1.5f;
	m_lightingSettings.SSAOBias = 0.082f;
}

void Scene::LoadSceneSkybox()
{
	auto lockedContext = Render::GetContext();
	//ThrowIfFailed(DirectX::CreateDDSTextureFromFile(Render::GetDevice(), lockedContext.GetContext(), (IOManager::ProjectDirectory / L"DiffuseHDR.dds").c_str(), nullptr, &m_radianceResourceView));
	//ThrowIfFailed(DirectX::CreateDDSTextureFromFile(Render::GetDevice(), lockedContext.GetContext(), (IOManager::ProjectDirectory / L"SpecularHDR.dds").c_str(), nullptr, &m_specularResourceView));
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile(Render::GetDevice(), lockedContext.GetContext(), (IOManager::ExecutableDirectory / L"brdf.dds").c_str(), nullptr, &m_specularIntegrationResourceView));
	//ThrowIfFailed(DirectX::CreateDDSTextureFromFile(Render::GetDevice(), lockedContext.GetContext(), (IOManager::ProjectDirectory / L"EnvHDR.dds").c_str(), nullptr, &m_skyboxResourceView));
}

Light* Scene::GetSun()
{
	const auto& gameObjects = ObjectManager::GetInstance()->GetGameObjects();
	for (const auto& gameObject : gameObjects)
	{
		if (const auto& lightComponent = gameObject->GetComponentOfType<LightComponent>())
		{
			if (lightComponent->GetLight()->Type == Light::LightType::Directional)
			{
				return lightComponent->GetLight().get();
			}
		}
	}
	return nullptr;
}
