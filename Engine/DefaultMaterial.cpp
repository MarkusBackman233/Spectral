#include "DefaultMaterial.h"
#include "Texture.h"
#include "ResourceManager.h"
#include "Json.h"
#include "Mesh.h"
#include "iRender.h"
#include <d3d11.h>
#include "InstanceManager.h"
#include "DeviceResources.h"

DefaultMaterial::MaterialGlobals DefaultMaterial::m_materialGlobals;

DefaultMaterial::DefaultMaterial() : m_textures{}
{
    m_settings.CombinedMaterialTexture = false;
    m_settings.BackfaceCulling = true;
    m_settings.LinearFiltering = true;
    m_settings.Roughness = 0.8f;
    m_settings.Metallic = 0.0f;
    m_settings.Color = Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}


void DefaultMaterial::Render(ID3D11DeviceContext* context, const DeviceResources& deviceResources, std::shared_ptr<Mesh> mesh, InstanceManager::InstanceData instanceData)
{
    Render::SetShaders(m_materialGlobals.m_pPixelShader, m_materialGlobals.m_pVertexShader, m_materialGlobals.m_pInputLayout, context);

    UINT stride = sizeof(Mesh::Vertex);
    UINT offset = 0;
    UINT strideInstance = sizeof(Math::Matrix);

    auto SetShaderResource = [&](DefaultMaterial::TextureType textureType, float& data, float fallbackValue = 1.0f)
    {
        if (GetTexture(textureType).get() && GetTexture(textureType)->GetResourceView().Get())
        {
            context->PSSetShaderResources(textureType, 1, GetTexture(textureType)->GetResourceView().GetAddressOf());
            data = -1.0f;
        }
        else
        {
            data = fallbackValue;
        }
    };

    SetShaderResource(DefaultMaterial::BaseColor, m_materialGlobals.m_pixelConstantBuffer.data2.x);
    SetShaderResource(DefaultMaterial::Normal, m_materialGlobals.m_pixelConstantBuffer.data2.y);
    SetShaderResource(DefaultMaterial::Roughness, m_materialGlobals.m_pixelConstantBuffer.data.y, m_settings.Roughness);
    SetShaderResource(DefaultMaterial::Metallic, m_materialGlobals.m_pixelConstantBuffer.data.z, m_settings.Metallic);
    SetShaderResource(DefaultMaterial::AmbientOcclusion, m_materialGlobals.m_pixelConstantBuffer.data.x);

    if (m_settings.BackfaceCulling)
    {
        context->RSSetState(deviceResources.GetBackfaceCullingRasterizer());
    }
    else
    {
        context->RSSetState(deviceResources.GetNoCullingRasterizer());
    }

    m_materialGlobals.m_pixelConstantBuffer.materialColor = m_settings.Color;
    m_materialGlobals.m_pixelConstantBuffer.data2.w = m_settings.CombinedMaterialTexture ? 1.0f : 0.0f;
    if (!m_settings.LinearFiltering)
    {
        ID3D11SamplerState* samplers[1] = {
            deviceResources.GetPointSamplerState()
        };
        context->PSSetSamplers(0, 1, samplers);
    }


    Render::UpdateConstantBuffer(Render::SHADER_TYPE_PIXEL, 1, m_materialGlobals.m_pPixelConstantBufferData, &m_materialGlobals.m_pixelConstantBuffer, context);

    context->IASetVertexBuffers(0, 1, mesh->m_pVertexBuffer.GetAddressOf(), &stride, &offset);

    context->IASetVertexBuffers(1, 1, instanceData.Buffer.GetAddressOf(), &strideInstance, &offset);
    context->IASetIndexBuffer(mesh->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->DrawIndexedInstanced(static_cast<UINT>(mesh->indices32.size()), instanceData.CurrentInstanceCount, 0, 0, 0);

    if (!m_settings.LinearFiltering)
    {
        ID3D11SamplerState* samplers[1] = {
            deviceResources.GetDefaultSamplerState()
        };
        context->PSSetSamplers(0, 1, samplers);
    }
    
}

void DefaultMaterial::CreateResources(ID3D11Device* device)
{

    D3D11_INPUT_ELEMENT_DESC vertexLayout[] = {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,     0,  0,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "COLOR",     0, DXGI_FORMAT_R8G8B8A8_UNORM,      0, 12,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,        0, 16,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,     0, 24,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT,     0, 36,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "Transform", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,  1,  0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "Transform", 1, DXGI_FORMAT_R32G32B32A32_FLOAT,  1, 16,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "Transform", 2, DXGI_FORMAT_R32G32B32A32_FLOAT,  1, 32,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "Transform", 3, DXGI_FORMAT_R32G32B32A32_FLOAT,  1, 48,  D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    };
    Render::CreateVertexShader(device, "GBuffer_VS.cso", &m_materialGlobals.m_pVertexShader, vertexLayout, ARRAYSIZE(vertexLayout), &m_materialGlobals.m_pInputLayout);
    Render::CreatePixelShader(device, "GBuffer_PS.cso", &m_materialGlobals.m_pPixelShader);

    Render::CreateConstantBuffer(device, sizeof(PixelConstantBuffer), m_materialGlobals.m_pPixelConstantBufferData);
}


bool DefaultMaterial::Load(const std::filesystem::path& file)
{
    auto json = Json::ParseFile(file);
    if (json.HasError())
    {
        SetTexture(0, ResourceManager::GetInstance()->GetResource<Texture>("TemplateGrid_albedo.bmp"));
        SetTexture(1, ResourceManager::GetInstance()->GetResource<Texture>("TemplateGrid_normal.bmp"));
        return true;
    }

    auto LoadTexture = [&](const char* textureName, DefaultMaterial::TextureType textureType)
    {
        if (json.Has(textureName))
        {
            std::string name = json[textureName].AsString();
            if (name == std::string("none") || name == std::string("None"))
            {
                return;
            }
            SetTexture(textureType, ResourceManager::GetInstance()->GetResource<Texture>(name));
        }
    };

    LoadTexture("Albedo", DefaultMaterial::BaseColor);
    LoadTexture("Normal", DefaultMaterial::Normal);
    LoadTexture("Roughness", DefaultMaterial::Roughness);
    LoadTexture("Metallic", DefaultMaterial::Metallic);
    LoadTexture("AmbientOcclusion", DefaultMaterial::AmbientOcclusion);

    if (json.Has("RougnessFloat"))
    {
        m_settings.Roughness = json["RougnessFloat"].AsFloat();
    }
    if (json.Has("MetallicFloat"))
    {
        m_settings.Metallic = json["MetallicFloat"].AsFloat();
    }
    if (json.Has("BackfaceCulling"))
    {
        m_settings.BackfaceCulling = json["BackfaceCulling"].AsBool();
    }
    if (json.Has("LinearFiltering"))
    {
        m_settings.LinearFiltering = json["LinearFiltering"].AsBool();
    }
    if (json.Has("CombinedMaterialTexture"))
    {
        m_settings.CombinedMaterialTexture = json["CombinedMaterialTexture"].AsBool();
    }
    if (json.Has("Color")) 
    {
        auto colorObject = json["Color"].AsArray();
        auto& color = m_settings.Color;
        color.x = colorObject[0].AsFloat();
        color.y = colorObject[1].AsFloat();
        color.z = colorObject[2].AsFloat();
        color.w = colorObject[3].AsFloat();
    }
    return true;
}



void DefaultMaterial::SetTexture(int index, std::shared_ptr<Texture> texture)
{
    m_textures[index] = texture;
}
