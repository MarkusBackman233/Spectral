#include "Thumbnail.h"
#include <iRender.h>
#include <RenderManager.h>
#include <DefaultMaterial.h>
#include <Texture.h>
#include <Mesh.h>
#include <ResourceManager.h>
#include <Model.h>

Thumbnail::GlobalThumbnailResources Thumbnail::m_resources;

void Thumbnail::CreateResources(ID3D11Device* device)
{
    D3D11_INPUT_ELEMENT_DESC vertexLayout[] = {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  0,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "COLOR",     0, DXGI_FORMAT_R8G8B8A8_UNORM,      0, 12,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,        0, 16,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,     0, 24,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT,     0, 36,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
    };

    Render::CreateVertexShader(device, "Forward_VS.cso", &m_resources.m_pVertexShader, vertexLayout, ARRAYSIZE(vertexLayout), &m_resources.m_pInputLayout);
    Render::CreatePixelShader(device, "PbrThumbnail_PS.cso", &m_resources.m_pPixelShader);

    Render::CreateConstantBuffer(device, sizeof(GlobalThumbnailResources::VertexConstantBuffer), m_resources.m_pVertexConstantBufferData);
    Render::CreateConstantBuffer(device, sizeof(GlobalThumbnailResources::PixelConstantBuffer), m_resources.m_pPixelConstantBufferData);

}

void Thumbnail::DestroyResources()
{

    m_resources.m_pInputLayout.Reset();
    m_resources.m_pVertexShader.Reset();
    m_resources.m_pPixelShader.Reset();
    m_resources.m_pVertexConstantBufferData.Reset();
    m_resources.m_pPixelConstantBufferData.Reset();
}

void Thumbnail::RenderSubMesh(Model* model, const SubMesh& subMesh, DefaultMaterial* material, ID3D11DeviceContext* context)
{
    if (subMesh.m_mesh)
    {
        UINT stride = sizeof(Mesh::Vertex);
        UINT offset = 0;

        DefaultMaterial* materialToUse = material ? material : model->GetMaterials()[subMesh.m_materialIndex].get();

        if (!materialToUse)
        {
            materialToUse = ResourceManager::GetInstance()->GetResource<DefaultMaterial>("Default.material").get();
        }

        if (!materialToUse)
        {
            return;
        }
        auto SetShaderResource = [&](DefaultMaterial::TextureType textureType, float& data, float fallbackValue = 1.0f)
        {
            if (materialToUse->GetTexture(textureType).get() && materialToUse->GetTexture(textureType)->GetResourceView().Get())
            {
                context->PSSetShaderResources(textureType, 1, materialToUse->GetTexture(textureType)->GetResourceView().GetAddressOf());
                data = -1.0f;
            }
            else
            {
                data = fallbackValue;
            }
        };

        SetShaderResource(DefaultMaterial::BaseColor, m_resources.m_pixelConstantBuffer.data2.x);
        SetShaderResource(DefaultMaterial::Normal, m_resources.m_pixelConstantBuffer.data2.y);
        SetShaderResource(DefaultMaterial::Roughness, m_resources.m_pixelConstantBuffer.data.y, materialToUse->GetMaterialSettings().Roughness);
        SetShaderResource(DefaultMaterial::Metallic, m_resources.m_pixelConstantBuffer.data.z, materialToUse->GetMaterialSettings().Metallic);
        SetShaderResource(DefaultMaterial::AmbientOcclusion, m_resources.m_pixelConstantBuffer.data.x);
        m_resources.m_pixelConstantBuffer.materialColor = materialToUse->GetMaterialSettings().Color;

        Render::UpdateConstantBuffer(Render::SHADER_TYPE_PIXEL, 1, m_resources.m_pPixelConstantBufferData, &m_resources.m_pixelConstantBuffer, context);


        m_resources.m_vertexConstantBuffer.modelPose = subMesh.m_localMatrix;
        Render::UpdateConstantBuffer(Render::SHADER_TYPE_VERTEX, 0, m_resources.m_pVertexConstantBufferData, &m_resources.m_vertexConstantBuffer, context);


        context->IASetVertexBuffers(0, 1, subMesh.m_mesh->m_pVertexBuffer.GetAddressOf(), &stride, &offset);
        context->IASetIndexBuffer(subMesh.m_mesh->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        context->DrawIndexed(static_cast<UINT>(subMesh.m_mesh->indices32.size()), 0, 0);
    }

    for (auto& subMesh : subMesh.m_submeshes)
    {
        RenderSubMesh(model, subMesh, material, context);
    }
}

void Thumbnail::Render(Model* mesh, DefaultMaterial* material)
{
    auto device = Render::GetDevice();
    
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthState;
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    
    device->CreateDepthStencilState(&dsDesc, depthState.GetAddressOf());
    
    Microsoft::WRL::ComPtr<ID3D11Texture2D>        depthTexture;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DSV;
    
    
    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = 512;
    depthDesc.Height = 512;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    
    device->CreateTexture2D(&depthDesc, nullptr, depthTexture.GetAddressOf());
    
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = depthDesc.Format;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = 0;
    device->CreateDepthStencilView(depthTexture.Get(), &dsvDesc, DSV.GetAddressOf());
    
    
    auto lockedContext = Render::GetContext();
    auto context = lockedContext.GetContext();
    
    D3D11_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(512);
    viewport.Height = static_cast<float>(512);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    context->RSSetViewports(1, &viewport);
    
    auto deviceResources = RenderManager::GetInstance()->GetDeviceResources();
    
    const float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
    context->OMSetBlendState(deviceResources->GetDefaultBlendState(), blendFactor, 0xffffffff);
    
    ID3D11RenderTargetView* renderTargets[1]{ m_RTV.Get() };
    context->OMSetRenderTargets(1, renderTargets, DSV.Get());
    const float clearColor[4] = { 0.15f, 0.15f, 0.15f, 0.15f };
    context->ClearRenderTargetView(m_RTV.Get(), clearColor);
    context->ClearDepthStencilView(DSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    
    Render::SetShaders(m_resources.m_pPixelShader, m_resources.m_pVertexShader, m_resources.m_pInputLayout, context);
    
    Math::Vector3 bbMin = mesh->GetBoundingBoxMin();
    Math::Vector3 bbMax = mesh->GetBoundingBoxMax();
    Math::Vector3 center = (bbMin + bbMax) * 0.5f;
    float radius = (bbMax - center).Length();
    
    OrthographicCamera camera(
        Math::Vector2(radius * 2, radius * 2),
        0.1f,
        radius * 10.0f);
    
    camera.GetWorldMatrix().LookAt({ 5.0f, 5.0f, 5.0f }, { 0.0f, 0.0f, 0.0f });
    camera.CreateViewAndPerspective();
    context->OMSetDepthStencilState(depthState.Get(), 1);
    m_resources.m_vertexConstantBuffer.viewProjection = camera.GetViewProjectionMatrix();
    m_resources.m_vertexConstantBuffer.cameraPos = Math::Vector4(camera.GetWorldMatrix().GetPosition(), 1.0f);
    Render::UpdateConstantBuffer(Render::SHADER_TYPE_VERTEX, 0, m_resources.m_pVertexConstantBufferData, &m_resources.m_vertexConstantBuffer, context);
    RenderSubMesh(mesh, mesh->m_root, material, context);
    ID3D11RenderTargetView* nulltarget[1]{ RenderManager::GetInstance()->GetDeviceResources()->GetBackBufferTarget() };
    context->OMSetRenderTargets(1, nulltarget, nullptr);

}

Thumbnail::Thumbnail(Model* mesh)
{
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = 512;
    textureDesc.Height = 512;
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
    Thumbnail::Render(mesh, nullptr);
}

Thumbnail::Thumbnail(DefaultMaterial* material)
{
    static auto sphereMesh = ResourceManager::GetInstance()->GetResource<Model>("Default Sphere").get();
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = 512;
    textureDesc.Height = 512;
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
    Thumbnail::Render(sphereMesh, material);


    

}

Thumbnail::~Thumbnail()
{
    m_texture.Reset();
    m_RTV.Reset();
    m_SRV.Reset();
}

std::unordered_map<std::string, std::shared_ptr<Thumbnail>> ThumbnailManager::m_thumbnails;

std::shared_ptr<Thumbnail> ThumbnailManager::GetThumbnail(Model* mesh)
{
    auto it = m_thumbnails.find(mesh->m_filename);
    
    if (it != m_thumbnails.end())
    {
        return it->second;
    }
    auto thumbnail = std::make_shared<Thumbnail>(mesh);
    m_thumbnails.emplace(mesh->m_filename, thumbnail);
    return thumbnail;
}

void ThumbnailManager::RegenerateThumbnail(Model* mesh)
{
    auto it = m_thumbnails.find(mesh->m_filename);
    if (it != m_thumbnails.end())
    {
        it->second->Render(mesh, nullptr);
    }
}

std::shared_ptr<Thumbnail> ThumbnailManager::GetThumbnail(DefaultMaterial* material)
{
    auto it = m_thumbnails.find(material->m_filename);
    
    if (it != m_thumbnails.end())
    {
        return it->second;
    }
    auto thumbnail = std::make_shared<Thumbnail>(material);
    m_thumbnails.emplace(material->m_filename, thumbnail);
    return thumbnail;
}

void ThumbnailManager::RegenerateThumbnail(DefaultMaterial* material)
{
    auto it = m_thumbnails.find(material->m_filename);

    if (it != m_thumbnails.end())
    {
        static auto sphereMesh = ResourceManager::GetInstance()->GetResource<Model>("Default Sphere").get();
        it->second->Render(sphereMesh, material);
    }
}
