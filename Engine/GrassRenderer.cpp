#include "GrassRenderer.h"
#include <d3d11.h>
#include "iRender.h"
#include "IOManager.h"
#include "DeviceResources.h"
#include <vector>
#include "TimeManager.h"
#include "SkyboxManager.h"

std::vector<GrassPatch> GrassRenderer::m_grassRenderQueue;

void GrassRenderer::CreateResources(ID3D11Device* device)
{
    D3D11_INPUT_ELEMENT_DESC vertexLayout[] = {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  0,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,        0, 12,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
    };

    Render::CreateVertexShader(device, "Grass_VS.cso", &m_materialGlobals.m_pVertexShader, vertexLayout, ARRAYSIZE(vertexLayout), &m_materialGlobals.m_pInputLayout);
    Render::CreatePixelShader(device, "Grass_GBuffer_PS.cso", &m_materialGlobals.m_pPixelShader);

    HRESULT hr = S_OK;
    auto buffer = Render::LoadShaderBytecode((IOManager::ExecutableDirectory / "shaders" / "Grass_GS.cso").string());
    hr = device->CreateGeometryShader(buffer.data(), buffer.size(), nullptr, &m_materialGlobals.m_pGeometryShader);
    ThrowIfFailed(hr);

    Render::CreateConstantBuffer(device, sizeof(PixelConstantBuffer), m_materialGlobals.m_pPixelConstantBufferData);
    Render::CreateConstantBuffer(device, sizeof(GeometryConstantBuffer), m_materialGlobals.m_pGeometryConstantBufferData);
}

void GrassRenderer::AddGrassPatch(const GrassPatch& patch)
{
    if (patch.NumGrassPositions)
    {
        m_grassRenderQueue.push_back(patch);
    }
}

void GrassRenderer::Render(ID3D11DeviceContext* context, const DeviceResources& deviceResources)
{
    Render::SetShaders(m_materialGlobals.m_pPixelShader, m_materialGlobals.m_pVertexShader, m_materialGlobals.m_pInputLayout, context);
    context->GSSetShader(m_materialGlobals.m_pGeometryShader.Get(), nullptr, 0);

    m_materialGlobals.m_geometryConstantBuffer.viewProjection = Render::GetViewProjectionMatrix();
    m_materialGlobals.m_geometryConstantBuffer.cameraPos = Math::Vector4(Render::GetCameraPosition(), 1.0f);
    m_materialGlobals.m_geometryConstantBuffer.elapsedTime = TimeManager::GetLifeTime();

    context->UpdateSubresource(m_materialGlobals.m_pGeometryConstantBufferData.Get(), 0, nullptr, &m_materialGlobals.m_geometryConstantBuffer, 0, 0);
    context->GSSetConstantBuffers(0, 1, m_materialGlobals.m_pGeometryConstantBufferData.GetAddressOf());

    UINT stride = sizeof(GrassVertexV1);
    UINT offset = 0;
    context->OMSetBlendState(deviceResources.GetDefaultBlendState(), Math::Vector4(0.0f, 0.0f, 0.0f, 0.0f).Data(), 0xffffffff);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    context->RSSetState(deviceResources.GetNoCullingRasterizer());

    for (GrassPatch& patch : m_grassRenderQueue)
    {
        if (patch.NumGrassPositions)
        {
            context->IASetVertexBuffers(0, 1, patch.GrassPositionBufferData.GetAddressOf(), &stride, &offset);
            context->Draw(patch.NumGrassPositions, 0);
        }
    }
    m_grassRenderQueue.clear();
    context->GSSetShader(nullptr, nullptr, 0);
    context->RSSetState(deviceResources.GetBackfaceCullingRasterizer());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}