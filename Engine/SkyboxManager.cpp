#include "SkyboxManager.h"
#include <d3d11.h>
#include "iRender.h"
#include "Mesh.h"
#include "SceneManager.h"
#include "Matrix.h"
#include "DirectXMath.h"
#include "DxMathUtils.h"
#include "TimeManager.h"
#include "ProfilerManager.h"
#include "Light.h"

using namespace DirectX;



SkyboxManager::SkyboxManager()
{
 
}


void SkyboxManager::CreateResources(ID3D11DeviceContext* context, ID3D11Device* device)
{
    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    ThrowIfFailed(device->CreateSamplerState(&sampDesc, &samplerState));
    samplerState.As(&m_cubeSamplerState);

    m_skyboxMesh = std::make_shared<Mesh>();
    m_skyboxMesh->vertexes = {
        { Math::Vector3(1.0f,  1.0f,  1.0f) },
        { Math::Vector3(1.0f,  1.0f, -1.0f) },
        { Math::Vector3(-1.0f,  1.0f, -1.0f) },
        { Math::Vector3(-1.0f,  1.0f,  1.0f) },
        { Math::Vector3(1.0f, -1.0f, -1.0f) },
        { Math::Vector3(-1.0f, -1.0f, -1.0f) },
        { Math::Vector3(-1.0f,  1.0f, -1.0f) },
        { Math::Vector3(1.0f,  1.0f, -1.0f) },
        { Math::Vector3(-1.0f, -1.0f, -1.0f) },
        { Math::Vector3(-1.0f, -1.0f,  1.0f) },
        { Math::Vector3(-1.0f,  1.0f,  1.0f) },
        { Math::Vector3(-1.0f,  1.0f, -1.0f) },
        { Math::Vector3(-1.0f, -1.0f,  1.0f) },
        { Math::Vector3(-1.0f, -1.0f, -1.0f) },
        { Math::Vector3(1.0f, -1.0f, -1.0f) },
        { Math::Vector3(1.0f, -1.0f,  1.0f) },
        { Math::Vector3(1.0f, -1.0f,  1.0f) },
        { Math::Vector3(1.0f, -1.0f, -1.0f) },
        { Math::Vector3(1.0f,  1.0f, -1.0f) },
        { Math::Vector3(1.0f,  1.0f,  1.0f) },
        { Math::Vector3(-1.0f, -1.0f,  1.0f) },
        { Math::Vector3(1.0f, -1.0f,  1.0f) },
        { Math::Vector3(1.0f,  1.0f,  1.0f) },
        { Math::Vector3(-1.0f,  1.0f,  1.0f) },
    };
    m_skyboxMesh->indices32 = { 0,1,2,0,2,3,4,5,6,4,6,7,8,9,10,8,10,11,12,13,14,12,14,15,16,17,18,16,18,19,20,21,22,20,22,23 };
    m_skyboxMesh->CreateVertexAndIndexBuffer(device);

    D3D11_INPUT_ELEMENT_DESC vertexLayout[] = {
        { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT      , 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };


    m_pixelConstantBuffer.skyboxColor = Math::Vector4(0.6f, 0.6f, 0.6f, 1.0f);
    Render::CreateConstantBuffer(device, sizeof(VertexConstantBuffer), m_pVertexConstantBufferData);
    Render::CreateConstantBuffer(device, sizeof(PixelConstantBuffer), m_pPixelConstantBufferData);
    Render::CreatePixelShader(device, "Skybox_PS.cso", &m_skyboxPixelShader);
    Render::CreatePixelShader(device, "SkyboxIrradiance_PS.cso", &m_irradiancePixelShader);
    Render::CreatePixelShader(device, "SkyboxSpecular_PS.cso", &m_specularPixelShader);
    Render::CreateVertexShader(device, "Skybox_VS.cso", &m_skyboxVertexShader, vertexLayout, ARRAYSIZE(vertexLayout), &m_pInputLayout);


    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    D3D11_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};

    rtBlendDesc.BlendEnable = TRUE;
    rtBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA; // Source blend factor
    rtBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // Destination blend factor
    rtBlendDesc.BlendOp = D3D11_BLEND_OP_ADD; // Blend operation
    rtBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE; // Alpha source blend factor
    rtBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO; // Alpha destination blend factor
    rtBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD; // Alpha blend operation
    rtBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    blendDesc.RenderTarget[0] = rtBlendDesc;

    HRESULT hr = device->CreateBlendState(&blendDesc, m_alphaMixState.GetAddressOf());

    m_skybox = CreateCubemap(context, device, 512u, 1u, true);
    m_irradiance = CreateCubemap(context, device, 32u, 1u);
    unsigned int mipLevels = static_cast<unsigned int>(std::floor(std::log2(128u))) + 1u;
    m_specular = CreateCubemap(context, device, 128u, mipLevels);
    m_lastSpecular = CreateCubemap(context, device, m_specular.Resolution, m_specular.NumMips);
}

void SkyboxManager::RenderSkybox(ID3D11DeviceContext* context, ID3D11RenderTargetView* renderTarget)
{
    ProfileFunction
    context->PSSetShaderResources(0, 1, m_skybox.Resource.GetAddressOf());
    Render::SetShaders(m_skyboxPixelShader, m_skyboxVertexShader, m_pInputLayout, context);
    DrawSkybox(context, renderTarget);
    ID3D11ShaderResourceView* nullView = nullptr;
    context->PSSetShaderResources(0, 1, &nullView);
}

void SkyboxManager::DrawSkybox(ID3D11DeviceContext* context, ID3D11RenderTargetView* renderTarget)
{
    context->OMSetRenderTargets(1, &renderTarget, nullptr);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    UINT stride = sizeof(Mesh::Vertex);
    UINT offset = 0;
    UINT strideInstance = sizeof(Math::Matrix);


    Render::UpdateConstantBuffer(Render::SHADER_TYPE_PIXEL, 0, m_pPixelConstantBufferData, &m_pixelConstantBuffer, context);
    context->IASetVertexBuffers(0, 1, m_skyboxMesh->m_pVertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_skyboxMesh->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->DrawIndexed(static_cast<UINT>(m_skyboxMesh->indices32.size()), 0, 0);
}

void SkyboxManager::CreateCubeMap(ID3D11DeviceContext* context, ID3D11Device* device)
{
    ProfileFunction
    bool sunHasChanged = false;

    Scene& scene = SceneManager::GetInstance()->GetCurrentScene();
    if (scene.GetSkyboxResourceView() != nullptr)
    {
        m_pixelConstantBuffer.skyboxColor.w = 0.0f;
    }
    else
    {
        if (auto* sun = scene.GetSun())
        {

            if (m_pixelConstantBuffer.skyboxColor.x != sun->Direction.x || m_pixelConstantBuffer.skyboxColor.x != sun->Direction.x || m_pixelConstantBuffer.skyboxColor.x != sun->Direction.x)
            {
                sunHasChanged = true;
                m_pixelConstantBuffer.data.z = 0.0f;
            }

            m_pixelConstantBuffer.skyboxColor.x = sun->Direction.x;
            m_pixelConstantBuffer.skyboxColor.y = sun->Direction.y;
            m_pixelConstantBuffer.skyboxColor.z = sun->Direction.z;
        }

        m_pixelConstantBuffer.skyboxColor.w = 1.0f;
        m_pixelConstantBuffer.data.x = TimeManager::GetLifeTime();
    }

    if (m_pixelConstantBuffer.data.z * 0.00694 > 8.4)
    {
        return;
    }

    XMVECTOR lookAt[6] = {
        XMVectorSet(1, 0, 0, 0),   // +X
        XMVectorSet(-1, 0, 0, 0),  // -X
        XMVectorSet(0, 1, 0, 0),   // +Y
        XMVectorSet(0, -1, 0, 0),  // -Y
        XMVectorSet(0, 0, 1, 0),   // +Z
        XMVectorSet(0, 0, -1, 0)   // -Z
    };

    XMVECTOR up[6] = {
        XMVectorSet(0, -1, 0, 0),  // +X
        XMVectorSet(0, -1, 0, 0),  // -X
        XMVectorSet(0, 0, 1, 0),   // +Y
        XMVectorSet(0, 0, -1, 0),  // -Y
        XMVectorSet(0, -1, 0, 0),  // +Z
        XMVectorSet(0, -1, 0, 0)   // -Z
    };
    D3D11_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(m_skybox.Resolution);
    viewport.Height = static_cast<float>(m_skybox.Resolution);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    m_pixelConstantBuffer.data.z += 1.0f;
    if (sunHasChanged)
    {


        context->RSSetViewports(1, &viewport);
        Render::SetShaders(m_skyboxPixelShader, m_skyboxVertexShader, m_pInputLayout, context);

        for (int i = 0; i < 6; ++i) 
        {
            XMMATRIX view = XMMatrixLookAtLH(XMVectorZero(), lookAt[i], up[i]);
            m_vertexConstantBuffer.viewProjection = Spectral::DxMathUtils::ToSp(view);
            Render::UpdateConstantBuffer(Render::SHADER_TYPE_VERTEX, 0, m_pVertexConstantBufferData, &m_vertexConstantBuffer, context);
            DrawSkybox(context, m_skybox.GetRenderTarget(i));
        }    

        context->OMSetRenderTargets(0, nullptr, nullptr);
        context->GenerateMips(m_skybox.Resource.Get());

    }

    context->PSSetShaderResources(0, 1, m_skybox.Resource.GetAddressOf());
    context->PSSetShaderResources(1, 1, m_lastSpecular.Resource.GetAddressOf());

    Render::SetShaders(m_irradiancePixelShader, m_skyboxVertexShader, m_pInputLayout, context);

    viewport.Width = static_cast<float>(m_irradiance.Resolution);
    viewport.Height = static_cast<float>(m_irradiance.Resolution);
    context->RSSetViewports(1, &viewport);
    //if (sunHasChanged)
    {
        float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // Blend factor for source alpha
        UINT sampleMask = 0xFFFFFFFF; // Enable all samples
        
        context->OMSetBlendState(m_alphaMixState.Get(), blendFactor, sampleMask);
        
        for (int i = 0; i < 6; ++i) {
            XMMATRIX view = XMMatrixLookAtLH(XMVectorZero(), lookAt[i], up[i]);
            m_vertexConstantBuffer.viewProjection = Spectral::DxMathUtils::ToSp(view);
            Render::UpdateConstantBuffer(Render::SHADER_TYPE_VERTEX, 0, m_pVertexConstantBufferData, &m_vertexConstantBuffer, context);
            DrawSkybox(context, m_irradiance.GetRenderTarget(i));
        }
    }
    Render::SetShaders(m_specularPixelShader, m_skyboxVertexShader, m_pInputLayout, context);

    for (unsigned int mip = 0; mip < m_specular.NumMips; ++mip)
    {
        unsigned int mipSize = m_specular.Resolution >> mip;
        viewport.Width = static_cast<float>(mipSize);
        viewport.Height = static_cast<float>(mipSize);
        context->RSSetViewports(1, &viewport);
        float roughness = (float)mip / (float)(m_specular.NumMips - 1);
        m_pixelConstantBuffer.data.y = roughness;
        for (int i = 0; i < 6; ++i) 
        {
            XMMATRIX view = XMMatrixLookAtLH(XMVectorZero(), lookAt[i], up[i]);
            m_vertexConstantBuffer.viewProjection = Spectral::DxMathUtils::ToSp(view);
            Render::UpdateConstantBuffer(Render::SHADER_TYPE_VERTEX, 0, m_pVertexConstantBufferData, &m_vertexConstantBuffer, context);
            DrawSkybox(context, m_specular.RenderTargets[mip][i].Get());
        }
    }
    ID3D11ShaderResourceView* nullView[2] = { nullptr };
    context->PSSetShaderResources(0, 2, nullView);
    for (unsigned int mip = 0; mip < m_specular.NumMips; ++mip)
    {
        for (int i = 0; i < 6; ++i) 
        {
            //Render::CopyRenderTarget(context, m_specular.RenderTargets[mip][i].Get(), m_lastSpecular.RenderTargets[mip][i].Get());
    
    
            ID3D11Resource* pSrcResource = nullptr;
            ID3D11Resource* pDstResource = nullptr;
            m_lastSpecular.RenderTargets[mip][i]->GetResource(&pDstResource);
            m_specular.RenderTargets[mip][i]->GetResource(&pSrcResource);
            context->CopySubresourceRegion(
                pDstResource,                               // Destination texture
                D3D11CalcSubresource(mip, i, m_specular.NumMips), // Destination subresource index
                0, 0, 0,                                    // Destination offset
                pSrcResource,                           // Source texture
                D3D11CalcSubresource(mip, i, m_specular.NumMips), // Source subresource index
                nullptr                                     // No source offset
            );
    
            pSrcResource->Release();
            pDstResource->Release();
        }
    }
}

SkyboxManager::Cubemap SkyboxManager::CreateCubemap(ID3D11DeviceContext* context, ID3D11Device* device, unsigned int resolution, unsigned int numMips, bool generateMips)
{
    Cubemap cubemap;

    cubemap.SetMips(generateMips ? (static_cast<UINT>(log2(resolution)) + 1u) : numMips);
    cubemap.Resolution = resolution;
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = resolution;
    texDesc.Height = resolution;
    texDesc.MipLevels = cubemap.NumMips;
    texDesc.ArraySize = 6;
    texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | (generateMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);

    device->CreateTexture2D(&texDesc, nullptr, &cubemap.Texture);

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = texDesc.Format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    for (unsigned int mip = 0; mip < numMips; mip++)
    {
        for (unsigned int face = 0; face < 6; ++face) 
        {
            rtvDesc.Texture2DArray.FirstArraySlice = face;
            rtvDesc.Texture2DArray.ArraySize = 1;
            rtvDesc.Texture2DArray.MipSlice = mip;
            device->CreateRenderTargetView(cubemap.Texture.Get(), &rtvDesc, cubemap.RenderTargets[mip][face].GetAddressOf());
        }
    }
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.MipLevels = texDesc.MipLevels;
    device->CreateShaderResourceView(cubemap.Texture.Get(), &srvDesc, cubemap.Resource.GetAddressOf());

    if (generateMips)
    {
        context->GenerateMips(cubemap.Resource.Get());
    }

    return cubemap;
}

