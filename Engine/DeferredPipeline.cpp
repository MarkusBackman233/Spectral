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


    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    device->CreateTexture2D(&textureDesc, nullptr, m_gBufferTextures[GBufferTexture::Albedo].GetAddressOf());

    textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    device->CreateTexture2D(&textureDesc, nullptr, m_gBufferTextures[GBufferTexture::Normal].GetAddressOf());
    device->CreateTexture2D(&textureDesc, nullptr, m_gBufferTextures[GBufferTexture::WorldPosition].GetAddressOf());    

    for (int i = 0; i < GBufferTexture::NumTextures; ++i)
    {
        device->CreateRenderTargetView(m_gBufferTextures[i].Get(), nullptr, m_gBufferRTVs[i].GetAddressOf());
        device->CreateShaderResourceView(m_gBufferTextures[i].Get(), nullptr, m_gBufferSRVs[i].GetAddressOf());
    }
}

void DeferredPipeline::RenderGBuffer(
    ID3D11DeviceContext* context,
    const DeviceResources& deviceResources,
    const InstanceManager& instanceManager,
    const ShadowManager& shadowmanager)
{
    ProfileFunction


    context->ClearDepthStencilView(deviceResources.GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    context->OMSetBlendState(deviceResources.GetDefaultBlendState(), Math::Vector4(0, 0, 0, 5).Data(), 0xffffffff);
    ID3D11RenderTargetView* renderTargets[GBufferTexture::NumTextures]{ m_gBufferRTVs[0].Get(), m_gBufferRTVs[1].Get(), m_gBufferRTVs[2].Get() };
    context->OMSetRenderTargets(GBufferTexture::NumTextures, renderTargets, deviceResources.GetDepthStencil());
    for (int i = 0; i < GBufferTexture::NumTextures; ++i)
    {
        context->ClearRenderTargetView(m_gBufferRTVs[i].Get(), Math::Vector4(0, 0, 0, 5).Data());
    }

    m_vertexConstantBuffer.viewProjection = Render::GetViewProjectionMatrix();
    m_vertexConstantBuffer.cameraPos = Math::Vector4(Render::GetCameraPosition(),1.0f);
    Render::UpdateConstantBuffer(Render::SHADER_TYPE_VERTEX, 0, m_pVertexConstantBufferData, &m_vertexConstantBuffer, context);


    const auto& instances = instanceManager.GetInstances();
    for (const auto& [instance, matrixes] : instances)
    {
        instance.Material->Render(context, deviceResources, instance.Mesh, matrixes);
    }
}

ID3D11ShaderResourceView* DeferredPipeline::GetSRV(GBufferTexture texture) const
{
    return m_gBufferSRVs[texture].Get();
}

void DeferredPipeline::ReleaseResources()
{
    for (int i = 0; i < GBufferTexture::NumTextures; ++i)
    {
        m_gBufferTextures[i]->Release();
        m_gBufferRTVs[i]->Release();
        m_gBufferSRVs[i]->Release();
    }
}
