#include "DeferredPipeline.h"
#include "iRender.h"
#include "DeviceResources.h"
#include "Mesh.h"
#include "Matrix.h"
#include "RenderManager.h"
#include "Texture.h"
#include "ShadowManager.h"
#include "Material.h"
#include "ProfilerManager.h"

DeferredPipeline::DeferredPipeline()
{
}

void DeferredPipeline::CreateResources(ID3D11Device* device, const Math::Vector2& windowSize)
{
    D3D11_INPUT_ELEMENT_DESC vertexLayout[] = {
        { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT      , 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT         , 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT      , 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",    0, DXGI_FORMAT_R32G32B32_FLOAT      , 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "Transform",  0, DXGI_FORMAT_R32G32B32A32_FLOAT   , 1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "Transform",  1, DXGI_FORMAT_R32G32B32A32_FLOAT   , 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "Transform",  2, DXGI_FORMAT_R32G32B32A32_FLOAT   , 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "Transform",  3, DXGI_FORMAT_R32G32B32A32_FLOAT   , 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    };

    Render::CreateVertexShader(device, "GBuffer_VS.cso", &m_pVertexShader, vertexLayout, ARRAYSIZE(vertexLayout), &m_pInputLayout);
    Render::CreatePixelShader(device, "GBuffer_PS.cso", &m_pPixelShader);

    Render::CreateConstantBuffer(device, sizeof(VertexConstantBuffer), m_pVertexConstantBufferData);
    Render::CreateConstantBuffer(device, sizeof(PixelConstantBuffer), m_pPixelConstantBufferData);

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

    textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    device->CreateTexture2D(&textureDesc, nullptr, m_gBufferTextures[GBufferTexture::Normal].GetAddressOf());
    device->CreateTexture2D(&textureDesc, nullptr, m_gBufferTextures[GBufferTexture::WorldPosition].GetAddressOf());    
    device->CreateTexture2D(&textureDesc, nullptr, m_gBufferTextures[GBufferTexture::LightPosition].GetAddressOf());

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
    auto windowSize = Render::GetWindowSize();

    D3D11_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(windowSize.x);
    viewport.Height = static_cast<float>(windowSize.y);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;

    context->RSSetViewports(1, &viewport);

    context->ClearDepthStencilView(deviceResources.GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->OMSetBlendState(deviceResources.GetDefaultBlendState(), Math::Vector4(0, 0, 0, 5).Data(), 0xffffffff);
    ID3D11RenderTargetView* renderTargets[GBufferTexture::NumTextures]{ m_gBufferRTVs[0].Get(), m_gBufferRTVs[1].Get(), m_gBufferRTVs[2].Get(), m_gBufferRTVs[3].Get() };
    context->OMSetRenderTargets(GBufferTexture::NumTextures, renderTargets, deviceResources.GetDepthStencil());
    for (int i = 0; i < GBufferTexture::NumTextures; ++i)
    {
        context->ClearRenderTargetView(m_gBufferRTVs[i].Get(), Math::Vector4(0, 0, 0, 5).Data());
    }

    Render::SetShaders(m_pPixelShader, m_pVertexShader, m_pInputLayout, context);
    m_vertexConstantBuffer.viewProjection = Render::GetViewProjectionMatrix();
    m_vertexConstantBuffer.lightMatrix = shadowmanager.GetShadowViewProjectionMatrix();
    m_vertexConstantBuffer.cameraPos = Math::Vector4(Render::GetCameraPosition(),1.0f);
    Render::UpdateConstantBuffer(Render::SHADER_TYPE_VERTEX, 0, m_pVertexConstantBufferData, &m_vertexConstantBuffer, context);

    UINT stride = sizeof(Mesh::Vertex);
    UINT offset = 0;
    UINT strideInstance = sizeof(Math::Matrix);

    auto SetShaderResource = [&](Material* material, Material::TextureType textureType, float& data, float fallbackValue = 1.0f)
    {
        if (material->GetTexture(textureType).get() && material->GetTexture(textureType)->GetResourceView().Get())
        {
            context->PSSetShaderResources(textureType, 1, material->GetTexture(textureType)->GetResourceView().GetAddressOf());
            data = -1.0f;
        }
        else
        {
            data = fallbackValue;
        }
    };


    const auto& instances = instanceManager.GetInstances();
    for (const auto& [instance, matrixes] : instances)
    {

        Material* material = instance.Material.get();
        SetShaderResource(material,Material::BaseColor, m_pixelConstantBuffer.data2.x);
        SetShaderResource(material,Material::Normal, m_pixelConstantBuffer.data2.y);
        SetShaderResource(material,Material::Roughness, m_pixelConstantBuffer.data.y, material->GetMaterialSettings().Roughness);
        SetShaderResource(material,Material::Metallic, m_pixelConstantBuffer.data.z, material->GetMaterialSettings().Metallic);
        SetShaderResource(material,Material::AmbientOcclusion, m_pixelConstantBuffer.data.x);

        if (material->GetMaterialSettings().BackfaceCulling)
        {
            context->RSSetState(deviceResources.GetBackfaceCullingRasterizer());
        }
        else
        {
            context->RSSetState(deviceResources.GetNoCullingRasterizer());
        }

        m_pixelConstantBuffer.materialColor = material->GetMaterialSettings().Color;
        if (!material->GetMaterialSettings().LinearFiltering)
        {
            ID3D11SamplerState* samplers[1] = {
                deviceResources.GetPointSamplerState()
            };
            context->PSSetSamplers(0, 1, samplers);
        }


        Render::UpdateConstantBuffer(Render::SHADER_TYPE_PIXEL, 1, m_pPixelConstantBufferData, &m_pixelConstantBuffer, context);

        context->IASetVertexBuffers(0, 1, instance.Mesh->m_pVertexBuffer.GetAddressOf(), &stride, &offset);
        context->IASetIndexBuffer(instance.Mesh->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        context->IASetVertexBuffers(1, 1, matrixes.Buffer.GetAddressOf(), &strideInstance, &offset);
        context->DrawIndexedInstanced(static_cast<UINT>(instance.Mesh->indices32.size()), matrixes.CurrentInstanceCount, 0, 0, 0);

        if (!material->GetMaterialSettings().LinearFiltering)
        {
            ID3D11SamplerState* samplers[1] = {
                deviceResources.GetDefaultSamplerState()
            };
            context->PSSetSamplers(0, 1, samplers);
        }
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
