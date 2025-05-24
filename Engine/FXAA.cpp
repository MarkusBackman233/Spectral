#include "FXAA.h"
#include "iRender.h"
#include "ProfilerManager.h"
#include "DeviceResources.h"
FXAA::FXAA()
    : PostProcessing()
{
}

void FXAA::CreateResources(ID3D11Device* device)
{
    Render::CreatePixelShader(device, "FXAA_PS.cso", &m_pixelShader);
    PostProcessing::CreatePostProcessingResources(device);
}

void FXAA::Process(ID3D11DeviceContext* context, const DeviceResources& deviceResources)
{
    ProfileFunction
    SetPostProcessingRenderTarget(context, deviceResources);
    SetVertexBuffer(context);
    Render::SetShaders(m_pixelShader, m_vertexShader, m_inputLayout, context);
    ID3D11ShaderResourceView* srv = deviceResources.RenderTargetSRV();
    context->PSSetShaderResources(20, 1, &srv);
    context->Draw(4, 0);
    ID3D11ShaderResourceView* nullView = nullptr;
    context->PSSetShaderResources(20, 1, &nullView);
    CopyPostProcessingToRenderTarget(context, deviceResources);
}
