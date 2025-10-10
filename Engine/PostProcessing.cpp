#include "PostProcessing.h"
#include "iRender.h"
#include "Vector2.h"
#include "RenderManager.h"

PostProcessing::PostProcessing()
{

}


void PostProcessing::CreatePostProcessingResources(ID3D11Device* device)
{
    std::vector<Vertex> vertices = {
        { Math::Vector3(-1.0f,  1.0f, 0.0f), Math::Vector2(0.0f, 0.0f) }, // 0
        { Math::Vector3(-1.0f, -1.0f, 0.0f), Math::Vector2(0.0f, 1.0f) }, // 2
        { Math::Vector3(1.0f,   1.0f, 0.0f), Math::Vector2(1.0f, 0.0f) }, // 1
        { Math::Vector3(1.0f,  -1.0f, 0.0f), Math::Vector2(1.0f, 1.0f) }, // 3
    };

    m_vertexBuffer = Render::CreateVertexBuffer(device, vertices);

    D3D11_INPUT_ELEMENT_DESC vertexLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };


    Render::CreateVertexShader(device,"Fullscreen_VS.cso", &m_vertexShader, vertexLayout, ARRAYSIZE(vertexLayout), &m_inputLayout);
}


void PostProcessing::SetVertexBuffer(ID3D11DeviceContext* context)
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}
