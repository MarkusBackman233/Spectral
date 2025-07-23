#include "TerrainMaterial.h"
#include "Texture.h"
#include "ResourceManager.h"
#include "Json.h"
#include "Mesh.h"
#include "iRender.h"
#include <d3d11.h>
#include "InstanceManager.h"
#include "DeviceResources.h"

TerrainMaterial::MaterialGlobals TerrainMaterial::m_materialGlobals;
Math::Vector3 TerrainMaterial::g_mouseRayHit;
float TerrainMaterial::g_brushSize;

TerrainMaterial::TerrainMaterial()
{
}


void TerrainMaterial::Render(ID3D11DeviceContext* context, const DeviceResources& deviceResources, std::shared_ptr<Mesh> mesh, InstanceManager::InstanceData instanceData)
{
    Render::SetShaders(m_materialGlobals.m_pPixelShader, m_materialGlobals.m_pVertexShader, m_materialGlobals.m_pInputLayout, context);
    context->RSSetState(deviceResources.GetBackfaceCullingRasterizer());

    m_materialGlobals.m_pixelConstantBuffer.RaycastHitPos.x = g_mouseRayHit.x;
    m_materialGlobals.m_pixelConstantBuffer.RaycastHitPos.y = g_mouseRayHit.y;
    m_materialGlobals.m_pixelConstantBuffer.RaycastHitPos.z = g_mouseRayHit.z;
    m_materialGlobals.m_pixelConstantBuffer.BrushSize = g_brushSize;


    auto& materialData = m_materialGlobals.m_pixelConstantBuffer.MaterialData;


    for (size_t i = 0; i < 4; i++)
    {


        size_t albedoIndex = Albedo0 + i;
        size_t normalIndex = Normal0 + i;
        size_t roughnessIndex = Roughness0 + i;
        if (m_materials[i] == nullptr)
        {
            materialData.set(albedoIndex, false);
            materialData.set(normalIndex, false);
            materialData.set(roughnessIndex, false);
            continue;
        }

        materialData.set(albedoIndex, m_materials[i]->GetTexture(DefaultMaterial::TextureType::BaseColor) != nullptr);
        materialData.set(normalIndex, m_materials[i]->GetTexture(DefaultMaterial::TextureType::Normal) != nullptr);
        materialData.set(roughnessIndex, m_materials[i]->GetTexture(DefaultMaterial::TextureType::Roughness) != nullptr);



        auto SetShaderResource = [&](size_t shaderTextureBitIndex, int materialTextureIndex)
        {
            if (materialData.test(shaderTextureBitIndex))
                context->PSSetShaderResources(shaderTextureBitIndex, 1, m_materials[i]->GetTexture(materialTextureIndex)->GetResourceView().GetAddressOf());
        };


        SetShaderResource(albedoIndex, DefaultMaterial::TextureType::BaseColor);
        SetShaderResource(normalIndex, DefaultMaterial::TextureType::Normal);
        SetShaderResource(roughnessIndex, DefaultMaterial::TextureType::Roughness);
    }




    Render::UpdateConstantBuffer(Render::SHADER_TYPE_PIXEL, 1, m_materialGlobals.m_pPixelConstantBufferData, &m_materialGlobals.m_pixelConstantBuffer, context);

    UINT stride = sizeof(Mesh::Vertex);
    UINT offset = 0;
    UINT strideInstance = sizeof(Math::Matrix);



    context->IASetVertexBuffers(0, 1, mesh->m_pVertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetVertexBuffers(1, 1, instanceData.Buffer.GetAddressOf(), &strideInstance, &offset);
    context->IASetIndexBuffer(mesh->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->DrawIndexedInstanced(static_cast<UINT>(mesh->indices32.size()), instanceData.CurrentInstanceCount, 0, 0, 0);
}

void TerrainMaterial::CreateResources(ID3D11Device* device)
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
    Render::CreatePixelShader(device, "Terrain_GBuffer_PS.cso", &m_materialGlobals.m_pPixelShader);

    Render::CreateConstantBuffer(device, sizeof(PixelConstantBuffer), m_materialGlobals.m_pPixelConstantBufferData);
}


bool TerrainMaterial::Load(const std::filesystem::path& file)
{
    return true;
}
