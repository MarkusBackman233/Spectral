#include "LineRenderer.h"
#include "iRender.h"
#include "DeviceResources.h"
#include "ProfilerManager.h"

void LineRenderer::CreateResources(ID3D11Device* device)
{
    D3D11_INPUT_ELEMENT_DESC vertexLayout[] = {
        { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR" ,     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,  D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    Render::CreateVertexShader(device, "Line_VS.cso", &m_pVertexShader, vertexLayout, ARRAYSIZE(vertexLayout), &m_pInputLayout);
    Render::CreatePixelShader(device, "Line_PS.cso", &m_pPixelShader);
    Render::CreateConstantBuffer(device, sizeof(Math::Matrix), m_pVertexConstantBufferData);
}

void LineRenderer::Render(ID3D11DeviceContext* context, const DeviceResources& deviceResources)
{
    ProfileFunction

    ID3D11RenderTargetView* renderTarget = deviceResources.GetBackBufferTarget();
    ID3D11DepthStencilView* depthStencil = deviceResources.GetDepthStencil();

    context->OMSetRenderTargets(1, &renderTarget, depthStencil);


    Render::UpdateConstantBuffer(Render::SHADER_TYPE_VERTEX, 0, m_pVertexConstantBufferData, &Render::GetViewProjectionMatrix(), context);
    Render::SetShaders(m_pPixelShader, m_pVertexShader, m_pInputLayout, context);

    if (!m_linesToDraw.empty())
    {
        auto vertexBuffer = Render::CreateVertexBuffer(deviceResources.GetDevice(), m_linesToDraw);
        UINT stride = sizeof(LineRenderer::Vertex);
        UINT offset = 0;
        context->OMSetBlendState(deviceResources.GetDefaultBlendState(), Math::Vector4(0.0f, 0.0f ,0.0f ,0.0f).Data(), 0xffffffff);
        context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        context->Draw(static_cast<int>(m_linesToDraw.size()), 0);
    }
    m_linesToDraw.clear();
}

void LineRenderer::AddLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Vector4& color)
{
    m_linesToDraw.emplace_back(start, color);
    m_linesToDraw.emplace_back(end, color);
}
