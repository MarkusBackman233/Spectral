#include "LineMaterial.h"
#include "pch.h"
#include "iRender.h"
#include "Vector3.h"

LineMaterial::LineMaterial()
{
    HRESULT hr = S_OK;

    D3D11_INPUT_ELEMENT_DESC vertexLayout[] = {
        { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    hr = Render::CreateVertexShader("LineVertexShader.cso", &m_pVertexShader, vertexLayout, ARRAYSIZE(vertexLayout), &m_pInputLayout);
    hr = Render::CreatePixelShader("LinePixelShader.cso", &m_pPixelShader);

    hr = Render::CreateConstantBuffer(sizeof(VertexConstantBuffer), m_pVertexConstantBufferData);
    hr = Render::CreateConstantBuffer(sizeof(PixelConstantBuffer), m_pPixelConstantBufferData);

    std::vector<uint32_t> indicies = {0,1,2,3,4,5};
    auto [bufferDesc,bufferData] = Render::CreateIndexBuffer(indicies);
    
    hr = Render::GetDevice()->CreateBuffer(&bufferDesc,&bufferData,&m_pIndexBuffer);

    m_pixelConstantBuffer.LineColor = DirectX::XMFLOAT4(1, 1, 1, 1);
    Render::UpdateConstantBuffer(Render::SHADER_TYPE_PIXEL, 0, m_pPixelConstantBufferData, &m_pixelConstantBuffer);
}

void LineMaterial::PreparePerMaterial()
{
    m_vertexConstantBuffer.viewProjection = Render::GetViewProjectionMatrix();
    Render::UpdateConstantBuffer(Render::SHADER_TYPE_VERTEX, 0, m_pVertexConstantBufferData, &m_vertexConstantBuffer);
    Render::SetShaders(m_pPixelShader, m_pVertexShader, m_pInputLayout);
    auto* sampler = Render::GetDefaultSamplerState();
    auto lockedContext = Render::GetContext();
    lockedContext.GetContext()->PSSetSamplers(0, 1, &sampler);
    lockedContext.GetContext()->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0); 
}

void LineMaterial::SetColor(const Math::Vector3& color)
{
    m_pixelConstantBuffer.LineColor = DirectX::XMFLOAT4(color.x, color.y, color.z, color.w);
    Render::UpdateConstantBuffer(Render::SHADER_TYPE_PIXEL, 0, m_pPixelConstantBufferData, &m_pixelConstantBuffer);
}
