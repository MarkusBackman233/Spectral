#include "PbrRender.h"
#include "iRender.h"
#include "DeferredPipeline.h"
#include "ShadowManager.h"
#include "SkyboxManager.h"
#include "SceneManager.h"
#include "SSAO.h"
#include "LightComponent.h"
#include "ProfilerManager.h"

PbrRender::PbrRender()
{

}

void PbrRender::CreateResources(ID3D11Device* device)
{
    Render::CreatePixelShader(device, "PBR_PS.cso", &m_pixelShader);
    Render::CreateConstantBuffer(device, sizeof(PixelConstantBuffer), m_pPixelConstantBufferData);
    PostProcessing::CreatePostProcessingResources(device);
}

void PbrRender::Process(
    ID3D11DeviceContext* context, 
    const DeviceResources& deviceResources, 
    const DeferredPipeline& deferredPipeline, 
    const SkyboxManager& skyboxManager, 
    const ShadowManager& shadowManager, 
    const SSAO& ssao
)
{
    ProfileFunction
    Scene& scene = SceneManager::GetInstance()->GetCurrentScene();
    ID3D11RenderTargetView* renderTarget = deviceResources.GetRenderTarget();
    context->OMSetRenderTargets(1, &renderTarget, nullptr);
    SetVertexBuffer(context);
    Render::SetShaders(m_pixelShader, m_vertexShader, m_inputLayout, context);
    const int nbResources = 9;
    ID3D11ShaderResourceView* resources[nbResources] = {
        deferredPipeline.GetSRV(DeferredPipeline::GBufferTexture::Albedo), 
        deferredPipeline.GetSRV(DeferredPipeline::GBufferTexture::Normal),
        deferredPipeline.GetSRV(DeferredPipeline::GBufferTexture::WorldPosition),
        deferredPipeline.GetSRV(DeferredPipeline::GBufferTexture::LightPosition),
        shadowManager.GetShadowTexture().Get(),
        skyboxManager.GetIrradianceCubemap(),
        skyboxManager.GetSpecularCubemap(),
        scene.GetSpecularIntegrationResourceView().Get(),
        ssao.GetSRV()
    };
    context->PSSetShaderResources(0, nbResources, resources);

    auto& lightSettings = scene.GetLightingSettings();

    m_pixelConstantBuffer.ambientLighting = lightSettings.AmbientLight;
    m_pixelConstantBuffer.fogColor = lightSettings.FogColor;
    m_pixelConstantBuffer.cameraPosition.x = Render::GetCameraPosition().x;
    m_pixelConstantBuffer.cameraPosition.y = Render::GetCameraPosition().y;
    m_pixelConstantBuffer.cameraPosition.z = Render::GetCameraPosition().z;
    m_pixelConstantBuffer.gamma.x = lightSettings.gamma;

    for (size_t i = 0; i < m_lightsToRender.size() && i < 50; i++)
    {
        Light* light = m_lightsToRender[i].get();

        m_pixelConstantBuffer.lights[i].position = Math::Vector4(light->Position, light->Attenuation);
        m_pixelConstantBuffer.lights[i].direction = Math::Vector4(light->Direction, 1.0f);
        m_pixelConstantBuffer.lights[i].color = light->Color.GetNormalizedColor();
        m_pixelConstantBuffer.lights[i].additionalData.x = static_cast<float>(light->Type);
        m_pixelConstantBuffer.lights[i].additionalData.y = light->Attenuation;
        m_pixelConstantBuffer.lights[i].additionalData.z = light->Enabled ? 1.0f : 0.0f;
    }

    m_pixelConstantBuffer.cameraPosition.w = static_cast<float>(std::min(m_lightsToRender.size(), 50ull));

    Render::UpdateConstantBuffer(Render::SHADER_TYPE_PIXEL, 0, m_pPixelConstantBufferData, &m_pixelConstantBuffer, context);
    context->Draw(4, 0);

    ID3D11ShaderResourceView* nullSRV[nbResources] = { nullptr };
    context->PSSetShaderResources(0, nbResources, nullSRV);
    m_lightsToRender.clear();
}

void PbrRender::RenderLight(std::shared_ptr<Light> light)
{
    m_lightsToRender.push_back(light);
}
