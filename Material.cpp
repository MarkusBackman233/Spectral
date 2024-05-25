#include "Material.h"
#include <fstream>
#include "Editor.h"
#include "Spectral.h"
#include "iRender.h"
#include <iostream>
#include "ShadowManager.h"

bool Material::HasSetupShader = false;
Microsoft::WRL::ComPtr<ID3D11InputLayout>       Material::m_pInputLayout;
Microsoft::WRL::ComPtr<ID3D11VertexShader>      Material::m_pVertexShader;
Microsoft::WRL::ComPtr<ID3D11PixelShader>       Material::m_pPixelShader;
Microsoft::WRL::ComPtr<ID3D11Buffer>            Material::m_pVertexConstantBufferData;
Microsoft::WRL::ComPtr<ID3D11Buffer>            Material::m_pPixelConstantBufferData;
Material::VertexConstantBuffer                  Material::m_vertexConstantBuffer;
Material::Material()
{
    if (HasSetupShader == false)
    {
        HasSetupShader = true;
        SetupShader();
    }



    m_materialSettings.Roughness = 0.8f;
    m_materialSettings.Metallic = 0.0f;
}

void Material::SetupShader()
{
    HRESULT hr = S_OK;

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

    hr = Render::CreateVertexShader("VertexShader.cso", &m_pVertexShader, vertexLayout, ARRAYSIZE(vertexLayout), &m_pInputLayout);
    hr = Render::CreatePixelShader("PBRPixelShader.cso", &m_pPixelShader);

    hr = Render::CreateConstantBuffer(sizeof(VertexConstantBuffer), m_pVertexConstantBufferData);
    hr = Render::CreateConstantBuffer(sizeof(PixelConstantBuffer), m_pPixelConstantBufferData);
}

void Material::PrepareMaterialGlobals()
{
    Render::SetShaders(m_pPixelShader, m_pVertexShader, m_pInputLayout);
    m_vertexConstantBuffer.viewProjection = Render::GetViewProjectionMatrix();
    m_vertexConstantBuffer.cameraPosition = DirectX::XMFLOAT4(&Render::GetCameraPosition().x);
    m_vertexConstantBuffer.directionLightViewProjection = ShadowManager::GetInstance()->GetShadowViewProjectionMatrix();
    Render::UpdateConstantBuffer(Render::SHADER_TYPE_VERTEX, 0, m_pVertexConstantBufferData, &m_vertexConstantBuffer);
    auto lockedContext = Render::GetContext();
    ID3D11SamplerState* samplers[2] = { Render::GetDefaultSamplerState() , ShadowManager::GetInstance()->GetShadowCompareSamplerState() };
    auto context = lockedContext.GetContext();
    context->PSSetSamplers(0, 2, samplers);
}

void Material::PreparePerMaterial()
{
    {
        auto lockedContext = Render::GetContext();
        auto context = lockedContext.GetContext();
        context->PSSetShaderResources(ALBEDO, 1, GetTexture(ALBEDO)->GetResourceView().GetAddressOf());              
        context->PSSetShaderResources(NORMAL, 1, GetTexture(NORMAL)->GetResourceView().GetAddressOf());
        
        if (GetTexture(ROUGHNESS).get() && GetTexture(ROUGHNESS)->GetResourceView().Get())
        {
            context->PSSetShaderResources(ROUGHNESS, 1, GetTexture(ROUGHNESS)->GetResourceView().GetAddressOf());
            m_pixelConstantBuffer.data.y = 1.0f;
        }
        else
        {
            m_pixelConstantBuffer.data.y = 0.0f;
        }
        
        if (GetTexture(METALLIC).get() && GetTexture(METALLIC)->GetResourceView().Get())
        {
            context->PSSetShaderResources(METALLIC, 1, GetTexture(METALLIC)->GetResourceView().GetAddressOf());
            m_pixelConstantBuffer.data.z = 1.0f;
        }
        else
        {
            m_pixelConstantBuffer.data.z = 0.0f;
        }
        
        if (GetTexture(AO).get() && GetTexture(AO)->GetResourceView().Get())
        {
            context->PSSetShaderResources(AO, 1, GetTexture(AO)->GetResourceView().GetAddressOf());
            m_pixelConstantBuffer.data.x = 0.0f;
        }
        else
        {
            m_pixelConstantBuffer.data.x = 1.0f;
        }
    }

    m_pixelConstantBuffer.ambientLighting = DirectX::XMFLOAT4(&Spectral::GetInstance()->GetAmbientLight().r);
    m_pixelConstantBuffer.fogColor = DirectX::XMFLOAT4(&Spectral::GetInstance()->GetFogColor().r);

    const auto& lights = Spectral::GetInstance()->GetLights();
    for (size_t i = 0; i < lights.size(); i++)
    {
        auto color = lights[i]->color.GetNormalizedColor();
        m_pixelConstantBuffer.lights[i].position = DirectX::XMFLOAT4(lights[i]->position.x, lights[i]->position.y, lights[i]->position.z, 0.0);
        m_pixelConstantBuffer.lights[i].color = DirectX::XMFLOAT4(color.r, color.g, color.b, color.a);
    }
    m_pixelConstantBuffer.data.w = (float)lights.size();




    Render::UpdateConstantBuffer(Render::SHADER_TYPE_PIXEL, 0, m_pPixelConstantBufferData, &m_pixelConstantBuffer);
}

void Material::SetTexture(int index, std::shared_ptr<Texture> texture)
{
    m_textures[index] = texture;
}
