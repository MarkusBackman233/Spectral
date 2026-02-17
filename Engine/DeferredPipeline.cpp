#include "DeferredPipeline.h"
#include "iRender.h"
#include "DeviceResources.h"
#include "Mesh.h"
#include "Matrix.h"
#include "InstanceManager.h"
#include "Texture.h"
#include "ShadowManager.h"
#include "DefaultMaterial.h"
#include "ProfilerManager.h"

DeferredPipeline::DeferredPipeline()
{
}

void DeferredPipeline::CreateResources(ID3D11Device* device, const Math::Vector2& windowSize)
{
    Render::CreateConstantBuffer(device, sizeof(VertexConstantBuffer), m_pVertexConstantBufferData);

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
    textureDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;

    device->CreateTexture2D(&textureDesc, nullptr, m_gBufferTexture.GetAddressOf());
    device->CreateRenderTargetView(m_gBufferTexture.Get(), nullptr, m_gBufferRTV.GetAddressOf());
    device->CreateShaderResourceView(m_gBufferTexture.Get(), nullptr, m_gBufferSRV.GetAddressOf());
}

void DeferredPipeline::RenderGBuffer(
    ID3D11DeviceContext* context,
    const DeviceResources& deviceResources,
    const InstanceManager& instanceManager,
    const ShadowManager& shadowmanager,
    Math::Vector2 viewportSize
    )
{
    ProfileFunction
    D3D11_VIEWPORT viewport{};
    viewport.Width = viewportSize.x;
    viewport.Height = viewportSize.y;
    viewport.MaxDepth = 1.0f;
    context->RSSetViewports(1, &viewport);
    context->ClearRenderTargetView(deviceResources.GetBackBufferTarget(), Math::Vector4(0, 0, 0, 1).Data());
    context->ClearDepthStencilView(deviceResources.GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->OMSetBlendState(deviceResources.GetDefaultBlendState(), Math::Vector4(0, 0, 0, 1).Data(), 0xffffffff);
    ID3D11RenderTargetView* renderTargets[1]{ m_gBufferRTV.Get() };
    context->OMSetRenderTargets(1, renderTargets, deviceResources.GetDepthStencil());
    context->ClearRenderTargetView(m_gBufferRTV.Get(), Math::Vector4(0, 0, 0, 0).Data());

    m_vertexConstantBuffer.viewProjection = Render::GetViewProjectionMatrix();
    m_vertexConstantBuffer.cameraPos = Math::Vector4(Render::GetCameraPosition(),1.0f);
    Render::UpdateConstantBuffer(Render::SHADER_TYPE_VERTEX, 0, m_pVertexConstantBufferData, &m_vertexConstantBuffer, context);


    const auto& instances = instanceManager.GetInstances();
    for (const auto& [instance, matrixes] : instances)
    {
        instance.Material->Render(context, deviceResources, instance.Mesh, matrixes);
    }
}

ID3D11ShaderResourceView* DeferredPipeline::GetSRV() const
{
    return m_gBufferSRV.Get();
}

void DeferredPipeline::ReleaseResources()
{
    m_gBufferTexture->Release();
    m_gBufferRTV->Release();
    m_gBufferSRV->Release();
}
