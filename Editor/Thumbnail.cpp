#include "Thumbnail.h"
#include <iRender.h>
#include <RenderManager.h>
#include <DefaultMaterial.h>
#include <Texture.h>
#include <Mesh.h>

Thumbnail::Thumbnail(Mesh* mesh, DefaultMaterial* material)
{
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = 128;
    textureDesc.Height = 128;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    auto device = Render::GetDevice();
    device->CreateTexture2D(&textureDesc, nullptr, m_texture.GetAddressOf());
    device->CreateRenderTargetView(m_texture.Get(), nullptr, m_RTV.GetAddressOf());
    device->CreateShaderResourceView(m_texture.Get(), nullptr, m_SRV.GetAddressOf());

    D3D11_INPUT_ELEMENT_DESC vertexLayout[] = {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  0,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "COLOR",     0, DXGI_FORMAT_R8G8B8A8_UNORM,      0, 12,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,        0, 16,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,     0, 24,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT,     0, 36,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
    };

    Render::CreateVertexShader(device, "Forward_VS.cso", &m_pVertexShader, vertexLayout, ARRAYSIZE(vertexLayout), &m_pInputLayout);
    Render::CreatePixelShader(device, "PbrThumbnail_PS.cso", &m_pPixelShader);

    Render::CreateConstantBuffer(device, sizeof(VertexConstantBuffer), m_pVertexConstantBufferData);
    Render::CreateConstantBuffer(device, sizeof(PixelConstantBuffer), m_pPixelConstantBufferData);

    auto lockedContext = Render::GetContext();
    auto context = lockedContext.GetContext();


    D3D11_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(128);
    viewport.Height = static_cast<float>(128);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    context->RSSetViewports(1, &viewport);

    auto deviceResources = RenderManager::GetInstance()->GetDeviceResources();

    const float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
    context->OMSetBlendState(deviceResources->GetDefaultBlendState(), blendFactor, 0xffffffff);

    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    context->ClearRenderTargetView(m_RTV.Get(), clearColor);

    ID3D11RenderTargetView* renderTargets[1]{ m_RTV.Get() };
    context->OMSetRenderTargets(1, renderTargets, nullptr);



    Render::SetShaders(m_pPixelShader, m_pVertexShader, m_pInputLayout, context);


    float size = mesh->GetBoundingBoxMax().Length()*2.0f+0.1f;

    OrthographicCamera camera(Math::Vector2(size, size),0.3f,100.0f);
    camera.GetWorldMatrix().SetPosition({ 0.0f, 0.0f, 5.0f });
    camera.CreateViewAndPerspective();

    m_vertexConstantBuffer.viewProjection = camera.GetViewProjectionMatrix();
    m_vertexConstantBuffer.cameraPos = Math::Vector4(camera.GetWorldMatrix().GetPosition(), 1.0f);
    Render::UpdateConstantBuffer(Render::SHADER_TYPE_VERTEX, 0, m_pVertexConstantBufferData, &m_vertexConstantBuffer, context);

    context->RSSetState(deviceResources->GetNoCullingRasterizer());

    ID3D11SamplerState* samplers[1] = { deviceResources->GetDefaultSamplerState() };
    context->PSSetSamplers(0, 1, samplers);
    UINT stride = sizeof(Mesh::Vertex);
    UINT offset = 0;







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

    SetShaderResource(DefaultMaterial::BaseColor, m_pixelConstantBuffer.data2.x);
    SetShaderResource(DefaultMaterial::Normal, m_pixelConstantBuffer.data2.y);
    SetShaderResource(DefaultMaterial::Roughness, m_pixelConstantBuffer.data.y, material->GetMaterialSettings().Roughness);
    SetShaderResource(DefaultMaterial::Metallic, m_pixelConstantBuffer.data.z, material->GetMaterialSettings().Metallic);
    SetShaderResource(DefaultMaterial::AmbientOcclusion, m_pixelConstantBuffer.data.x);


    Render::UpdateConstantBuffer(Render::SHADER_TYPE_PIXEL, 1, m_pPixelConstantBufferData, &m_pixelConstantBuffer, context);

    context->IASetVertexBuffers(0, 1, mesh->m_pVertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(mesh->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->DrawIndexed(static_cast<UINT>(mesh->indices32.size()), 0, 0);
}

Thumbnail::~Thumbnail()
{
    m_texture.Reset();
    m_RTV.Reset();
    m_SRV.Reset();
    m_pInputLayout.Reset();
    m_pVertexShader.Reset();
    m_pPixelShader.Reset();
    m_pVertexConstantBufferData.Reset();
    m_pPixelConstantBufferData.Reset();
}
