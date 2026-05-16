#include "BillboardRenderer.h"
#include <d3d11.h>
#include "iRender.h"
#include "IOManager.h"
#include "DeviceResources.h"
#include <vector>
#include "TimeManager.h"
#include "SkyboxManager.h"
#include "DefaultMaterial.h"
#include "Texture.h"
std::unordered_map<std::shared_ptr<DefaultMaterial>, std::vector<BillboardRenderData>> BillboardRenderer::m_renderQueue;

void BillboardRenderer::CreateResources(ID3D11Device* device)
{
    D3D11_INPUT_ELEMENT_DESC vertexLayout[] = {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  0,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "SIZE",      0, DXGI_FORMAT_R32_FLOAT,        0, 12,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "Transform", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,  1,  0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "Transform", 1, DXGI_FORMAT_R32G32B32A32_FLOAT,  1, 16,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "Transform", 2, DXGI_FORMAT_R32G32B32A32_FLOAT,  1, 32,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "Transform", 3, DXGI_FORMAT_R32G32B32A32_FLOAT,  1, 48,  D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    };

    Render::CreateVertexShader(device, "Billboard_VS.cso", &m_materialGlobals.m_pVertexShader, vertexLayout, ARRAYSIZE(vertexLayout), &m_materialGlobals.m_pInputLayout);
    Render::CreatePixelShader(device, "Billboard_GBuffer_PS.cso", &m_materialGlobals.m_pPixelShader);

    HRESULT hr = S_OK;
    auto buffer = Render::LoadShaderBytecode((IOManager::ExecutableDirectory / "shaders" / "Billboard_GS.cso").string());
    hr = device->CreateGeometryShader(buffer.data(), buffer.size(), nullptr, &m_materialGlobals.m_pGeometryShader);
    ThrowIfFailed(hr);

    Render::CreateConstantBuffer(device, sizeof(PixelConstantBuffer), m_materialGlobals.m_pPixelConstantBufferData);
    Render::CreateConstantBuffer(device, sizeof(GeometryConstantBuffer), m_materialGlobals.m_pGeometryConstantBufferData);
}

void BillboardRenderer::AddBillboard(std::shared_ptr<DefaultMaterial> material, const BillboardRenderData& renderData)
{
    m_renderQueue[material].push_back(renderData);
}


void BillboardRenderer::Render(ID3D11DeviceContext* context, const DeviceResources& deviceResources)
{
    Render::SetShaders(m_materialGlobals.m_pPixelShader, m_materialGlobals.m_pVertexShader, m_materialGlobals.m_pInputLayout, context);
    context->GSSetShader(m_materialGlobals.m_pGeometryShader.Get(), nullptr, 0);

    m_materialGlobals.m_geometryConstantBuffer.viewProjection = Render::GetViewProjectionMatrix();
    m_materialGlobals.m_geometryConstantBuffer.cameraPos = Math::Vector4(Render::GetCameraPosition(), 1.0f);
    m_materialGlobals.m_geometryConstantBuffer.elapsedTime = TimeManager::GetLifeTime();

    context->UpdateSubresource(m_materialGlobals.m_pGeometryConstantBufferData.Get(), 0, nullptr, &m_materialGlobals.m_geometryConstantBuffer, 0, 0);
    context->GSSetConstantBuffers(0, 1, m_materialGlobals.m_pGeometryConstantBufferData.GetAddressOf());

    m_materialGlobals.m_pixelConstantBuffer.cameraForward = Render::GetCamera()->GetWorldMatrix().GetFront();
    m_materialGlobals.m_pixelConstantBuffer.cameraRight = -Render::GetCamera()->GetWorldMatrix().GetLeft();
    m_materialGlobals.m_pixelConstantBuffer.cameraUp = Render::GetCamera()->GetWorldMatrix().GetUp();

    context->UpdateSubresource(m_materialGlobals.m_pPixelConstantBufferData.Get(), 0, nullptr, &m_materialGlobals.m_geometryConstantBuffer, 0, 0);
    context->PSSetConstantBuffers(0, 1,m_materialGlobals.m_pPixelConstantBufferData.GetAddressOf());

    UINT stride = sizeof(BillboardVertex);
    UINT instanceStride = sizeof(Math::Matrix);
    UINT offset = 0;
    context->OMSetBlendState(deviceResources.GetDefaultBlendState(), Math::Vector4(0.0f, 0.0f, 0.0f, 0.0f).Data(), 0xffffffff);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    context->RSSetState(deviceResources.GetNoCullingRasterizer());

    for (auto& [material, renderDatas]: m_renderQueue)
    {
        auto SetShaderResource = [&](DefaultMaterial::TextureType textureType, float& data, float fallbackValue = 1.0f)
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

        SetShaderResource(DefaultMaterial::BaseColor, material->m_materialGlobals.m_pixelConstantBuffer.data2.x);
        SetShaderResource(DefaultMaterial::Normal, material->m_materialGlobals.m_pixelConstantBuffer.data2.y);
        SetShaderResource(DefaultMaterial::Roughness, material->m_materialGlobals.m_pixelConstantBuffer.data.y, material->GetMaterialSettings().Roughness);
        SetShaderResource(DefaultMaterial::Metallic, material->m_materialGlobals.m_pixelConstantBuffer.data.z, material->GetMaterialSettings().Metallic);
        SetShaderResource(DefaultMaterial::AmbientOcclusion, material->m_materialGlobals.m_pixelConstantBuffer.data.x);

        if (!renderDatas.empty())
        {
            for (const BillboardRenderData& renderData : renderDatas)
            {
                context->IASetVertexBuffers(0, 1, renderData.m_vertexData.Buffer.GetAddressOf(), &stride, &offset);
                context->IASetVertexBuffers(1, 1, renderData.m_instanceData.Buffer.GetAddressOf(), &instanceStride, &offset);
                context->DrawInstanced(renderData.m_vertexData.NbVertices,static_cast<UINT>(renderData.m_instanceData.CurrentInstanceCount),0, 0);
            }
        }
    }
    m_renderQueue.clear();
    context->GSSetShader(nullptr, nullptr, 0);
    context->RSSetState(deviceResources.GetBackfaceCullingRasterizer());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}