#include "PbrRender.h"
#include "iRender.h"
#include "DeferredPipeline.h"
#include "ShadowManager.h"
#include "SkyboxManager.h"
#include "SceneManager.h"
#include "SSAO.h"
#include "ProfilerManager.h"
#include "DeviceResources.h"
#include "Light.h"

#include <random>

PbrRender::PbrRender()
{

}

void PbrRender::CreateResources(ID3D11Device* device)
{
    Render::CreatePixelShader(device, "PBR_PS.cso", &m_pixelShader);
    Render::CreateConstantBuffer(device, sizeof(PixelConstantBuffer), m_pPixelConstantBufferData);
    Render::CreateConstantBuffer(device, sizeof(SSAOPixelConstantBuffer), m_pSSAOPixelConstantBufferData);
    PostProcessing::CreatePostProcessingResources(device);
    /*
    std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f); // random floats between [0.0, 1.0]
    std::default_random_engine generator;

    auto Lerp = [&](float a, float b, float f) {return a + f * (b - a); };

    for (unsigned int i = 0; i < 64; ++i)
    {

        Math::Vector3 sample(randomFloats(generator) * 2.0f - 1.0f,
            randomFloats(generator) * 2.0f - 1.0f,
            randomFloats(generator));

        sample = sample.GetNormal();
        sample *= randomFloats(generator);
        float scale = (float)i / 64.0f;
        scale = Lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;


        m_SSAOpixelConstantBuffer.kernelPosition[i] = Math::Vector4(sample, 1.0f);
    }
    */

    std::uniform_real_distribution<float> randomFloats(-1.0f, 1.0f); // random floats between [0.0, 1.0]
    std::default_random_engine generator;

    auto Lerp = [&](float a, float b, float f) {return a + f * (b - a); };

    for (unsigned int i = 0; i < 64; ++i)
    {

        Math::Vector3 sample(
            randomFloats(generator),
            randomFloats(generator),
            randomFloats(generator)
        );

        m_SSAOpixelConstantBuffer.kernelPosition[i] = Math::Vector4(sample, 1.0f);
    }
}

void PbrRender::Process(
    ID3D11DeviceContext* context, 
    const DeviceResources& deviceResources, 
    const DeferredPipeline& deferredPipeline, 
    const SkyboxManager& skyboxManager, 
    const ShadowManager& shadowManager
)
{
    ProfileFunction
    Scene& scene = SceneManager::GetInstance()->GetCurrentScene();
    ID3D11RenderTargetView* renderTarget = deviceResources.GetRenderTarget();
    context->OMSetRenderTargets(1, &renderTarget, nullptr);
    SetVertexBuffer(context);
    Render::SetShaders(m_pixelShader, m_vertexShader, m_inputLayout, context);
    const int nbResources = 8;
    ID3D11ShaderResourceView* resources[nbResources] = {
        deferredPipeline.GetSRV(), 
        deviceResources.GetDepthSRV(),
        nullptr,
        shadowManager.GetShadowTexture().Get(),
        skyboxManager.GetIrradianceCubemap(),
        skyboxManager.GetSpecularCubemap(),
        scene.GetSpecularIntegrationResourceView().Get()
    };
    context->PSSetShaderResources(0, nbResources, resources);

    auto& lightSettings = scene.GetLightingSettings();

    m_pixelConstantBuffer.Projection = Render::GetProjectionMatrix();
    m_pixelConstantBuffer.viewProjection = Render::GetViewProjectionMatrix();
    m_pixelConstantBuffer.viewProjectionInverse = (Render::GetViewMatrix() * Render::GetProjectionMatrix()).GetInverse();
    m_pixelConstantBuffer.ProjectionInverse = Render::GetProjectionMatrix().GetInverse();
    m_pixelConstantBuffer.lightMatrix = shadowManager.GetShadowViewProjectionMatrix();
    m_pixelConstantBuffer.ambientLighting = lightSettings.AmbientLight;
    m_pixelConstantBuffer.fogColor = lightSettings.FogColor;
    m_pixelConstantBuffer.cameraPosition.x = Render::GetCameraPosition().x;
    m_pixelConstantBuffer.cameraPosition.y = Render::GetCameraPosition().y;
    m_pixelConstantBuffer.cameraPosition.z = Render::GetCameraPosition().z;
    m_pixelConstantBuffer.gamma.x = lightSettings.gamma;

    for (size_t i = 0; i < m_lightsToRender.size() && i < 50; i++)
    {
        const Light* light = m_lightsToRender[i];
    
        m_pixelConstantBuffer.lights[i].position = Math::Vector4(light->Position, light->Attenuation);
        m_pixelConstantBuffer.lights[i].direction = Math::Vector4(light->Direction, 1.0f);
        m_pixelConstantBuffer.lights[i].color = light->Color.GetNormalizedColor();
        m_pixelConstantBuffer.lights[i].additionalData.x = static_cast<float>(light->Type);
        m_pixelConstantBuffer.lights[i].additionalData.y = light->Attenuation;
        //m_pixelConstantBuffer.lights[i].additionalData.z = light->Enabled ? 1.0f : 0.0f;
    }

    m_pixelConstantBuffer.numLights = static_cast<uint32_t>(std::min(m_lightsToRender.size(), 50ull));

    Render::UpdateConstantBuffer(Render::SHADER_TYPE_PIXEL, 0, m_pPixelConstantBufferData, &m_pixelConstantBuffer, context);


    const auto& ssaoSettings = SceneManager::GetInstance()->GetCurrentScene().GetLightingSettings();
    m_SSAOpixelConstantBuffer.settings.x = ssaoSettings.SSAOBias;
    m_SSAOpixelConstantBuffer.settings.y = ssaoSettings.SSAOIntensity;
    m_SSAOpixelConstantBuffer.settings.z = ssaoSettings.SSAORadius;
    m_SSAOpixelConstantBuffer.settings.w = (float)rand();

    Render::UpdateConstantBuffer(Render::SHADER_TYPE_PIXEL, 5, m_pSSAOPixelConstantBufferData, &m_SSAOpixelConstantBuffer, context);

    context->Draw(4, 0);

    ID3D11ShaderResourceView* nullSRV[nbResources] = { nullptr };
    context->PSSetShaderResources(0, nbResources, nullSRV);
    m_lightsToRender.clear();
}

void PbrRender::RenderLight(const Light* light)
{
    m_lightsToRender.push_back(light);
}
