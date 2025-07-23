#include "GuizmoRenderer.h"
#include "pch.h"
#include "iRender.h"
#include "Vector3.h"
#include "Matrix.h"
#include "ProfilerManager.h"
#include "Texture.h"
#include "DeviceResources.h"
GuizmoRenderer::GuizmoRenderer()
{
 
}

void GuizmoRenderer::CreateResources(ID3D11Device* device)
{
    D3D11_INPUT_ELEMENT_DESC vertexLayout[] = {
        { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT      , 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT         , 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    Render::CreateVertexShader(device, "Guizmo_VS.cso", &m_pVertexShader, vertexLayout, ARRAYSIZE(vertexLayout), &m_pInputLayout);
    Render::CreatePixelShader(device, "Guizmo_PS.cso", &m_pPixelShader);

    Render::CreateConstantBuffer(device, sizeof(Math::Matrix), m_pVertexConstantBufferData);
    Render::CreateConstantBuffer(device, sizeof(Math::Vector4), m_pPixelConstantBufferData);

    std::vector<GuizmoRenderer::Vertex> vertices =
    { 
        { Math::Vector3(-0.5f, -0.5f, 0.0f), Math::Vector2(0.0f, 1.0f) },
        { Math::Vector3(0.5f, -0.5f, 0.0f), Math::Vector2(1.0f, 1.0f) },
        { Math::Vector3(0.5f,  0.5f, 0.0f), Math::Vector2(1.0f, 0.0f) },
        { Math::Vector3(-0.5f,  0.5f, 0.0f), Math::Vector2(0.0f, 0.0f) } 
    };

    m_pVertexBuffer = Render::CreateVertexBuffer(device,vertices);
    m_pIndexBuffer = Render::CreateIndexBuffer(device, std::vector<uint32_t>{0, 1, 2, 0, 2, 3});
}

void GuizmoRenderer::Render(ID3D11DeviceContext* context, const DeviceResources& deviceResources)
{
    ProfileFunction
    context->ClearDepthStencilView(deviceResources.GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->RSSetState(deviceResources.GetNoCullingRasterizer());
    for (const auto& [texture, guizmos] : m_guizmosToDraw)
    {
        for (const auto& [position, color] : guizmos)
        {
            Render::UpdateConstantBuffer(Render::SHADER_TYPE_PIXEL, 0, m_pPixelConstantBufferData, &color, context);
            Render::UpdateConstantBuffer(Render::SHADER_TYPE_VERTEX, 0, m_pVertexConstantBufferData, &Render::GetViewProjectionMatrix(), context);
            Render::SetShaders(m_pPixelShader, m_pVertexShader, m_pInputLayout, context);

            UINT stride = sizeof(GuizmoRenderer::Vertex);
            UINT offset = 0;

            auto camMat = Render::GetCamera()->GetWorldMatrix();
            camMat.SetPosition(Math::Vector3(0.0f, 0.0f, 0.0f));

            Math::Vector3 corner1 = position + Math::Vector3(-0.5f, -0.5f, 0.0f).Transform(camMat);
            Math::Vector3 corner2 = position + Math::Vector3(0.5f, -0.5f, 0.0f).Transform(camMat);
            Math::Vector3 corner3 = position + Math::Vector3(0.5f,  0.5f, 0.0f).Transform(camMat);
            Math::Vector3 corner4 = position + Math::Vector3(-0.5f,  0.5f, 0.0f).Transform(camMat);


            const std::vector<GuizmoRenderer::Vertex> vertices = {
                { corner1, Math::Vector2(0.0f, 1.0f) },
                { corner2, Math::Vector2(1.0f, 1.0f) },
                { corner3, Math::Vector2(1.0f, 0.0f) },
                { corner4, Math::Vector2(0.0f, 0.0f) },
            };


            m_pVertexBuffer = Render::CreateVertexBuffer(deviceResources.GetDevice(), vertices);
            context->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
            context->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            context->PSSetShaderResources(0, 1, texture->GetResourceView().GetAddressOf());
            context->DrawIndexed(6, 0, 0);
            m_pVertexBuffer.Reset();
        }

    }

    m_guizmosToDraw.clear();
    context->RSSetState(deviceResources.GetBackfaceCullingRasterizer());
}

void GuizmoRenderer::AddGuizmo(std::shared_ptr<Texture> texture, const Math::Vector3& position, const Math::Vector4& color)
{
    m_guizmosToDraw[texture].emplace_back(position, color);
}