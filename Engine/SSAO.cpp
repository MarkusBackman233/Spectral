#include "SSAO.h"
#include "iRender.h"
#include "SceneManager.h"
#include "Scene.h"
#include "DeferredPipeline.h"
#include <random>
#include "ProfilerManager.h"
#include "DeviceResources.h"

SSAO::SSAO()
    : PostProcessing()
{

}

void SSAO::CreateResources(ID3D11Device* device, const Math::Vector2& windowSize)
{
    Render::CreatePixelShader(device, "SSAO_PS.cso", &m_pixelShader);
    Render::CreateConstantBuffer(device, sizeof(PixelConstantBuffer), m_pPixelConstantBufferData);
    PostProcessing::CreatePostProcessingResources(device);


    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = static_cast<UINT>(windowSize.x);
    textureDesc.Height = static_cast<UINT>(windowSize.y);
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;
    textureDesc.Format = DXGI_FORMAT_R8_UNORM;

    device->CreateTexture2D(&textureDesc, nullptr, m_SSAOTexture.GetAddressOf());
    device->CreateRenderTargetView(m_SSAOTexture.Get(), nullptr, m_SSAORTV.GetAddressOf());
    device->CreateShaderResourceView(m_SSAOTexture.Get(), nullptr, m_SSAOSRV.GetAddressOf());



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


        m_pixelConstantBuffer.kernelPosition[i] = Math::Vector4(sample, 1.0f);
    }
}

void SSAO::Process(ID3D11DeviceContext* context, const DeviceResources& deviceResources, const DeferredPipeline& deferredPipeline)
{
    ProfileFunction
    ID3D11RenderTargetView* postPorcessingRenderTarget[] = { m_SSAORTV.Get()};
    context->OMSetRenderTargets(1, postPorcessingRenderTarget, nullptr);
    SetVertexBuffer(context);
    Render::SetShaders(m_pixelShader, m_vertexShader, m_inputLayout, context);

    std::vector<ID3D11ShaderResourceView*> resources = {
        deferredPipeline.GetSRV(),
        nullptr, // was worldpos
        deviceResources.GetDepthSRV(),
    };
    context->PSSetShaderResources(0, 3, resources.data());

    m_pixelConstantBuffer.viewProjectionMatrix = Render::GetViewProjectionMatrix();



    const auto& ssaoSettings = SceneManager::GetInstance()->GetCurrentScene().GetLightingSettings();
    m_pixelConstantBuffer.settings.x = ssaoSettings.SSAOBias;
    m_pixelConstantBuffer.settings.y = ssaoSettings.SSAOIntensity;
    m_pixelConstantBuffer.settings.z = ssaoSettings.SSAORadius;
    m_pixelConstantBuffer.settings.w = (float)rand();

    Render::UpdateConstantBuffer(Render::SHADER_TYPE_PIXEL, 0, m_pPixelConstantBufferData, &m_pixelConstantBuffer, context);

    context->Draw(4, 0);
}

ID3D11ShaderResourceView* SSAO::GetSRV() const
{
    return m_SSAOSRV.Get();
}

void SSAO::ReleaseResources()
{
	m_SSAOTexture->Release();
	m_SSAORTV->Release();
	m_SSAOSRV->Release();
}