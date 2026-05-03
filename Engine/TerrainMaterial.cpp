#include "TerrainMaterial.h"
#include "Texture.h"
#include "ResourceManager.h"
#include "Json.h"
#include "Mesh.h"
#include "iRender.h"
#include <d3d11.h>
#include "InstanceManager.h"
#include "DeviceResources.h"
#include "TerrainComponent.h"
#include <DirectXCollision.h>
#include "DxMathUtils.h"
#include "RenderManager.h"
TerrainMaterial::MaterialGlobals TerrainMaterial::m_materialGlobals;
Math::Vector3 TerrainMaterial::g_mouseRayHit;
float TerrainMaterial::g_brushSize;

TerrainMaterial::TerrainMaterial()
{
}


void TerrainMaterial::CreateResources(ID3D11Device* device)
{
    D3D11_INPUT_ELEMENT_DESC vertexLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "POSITIONINST", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0,D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    };

    Render::CreateVertexShader(device, "Terrain_GBuffer_VS.cso", &m_materialGlobals.m_pVertexShader, vertexLayout, ARRAYSIZE(vertexLayout), &m_materialGlobals.m_pInputLayout);
    Render::CreatePixelShader(device, "Terrain_GBuffer_PS.cso", &m_materialGlobals.m_pPixelShader);

    Render::CreateConstantBuffer(device, sizeof(PixelConstantBuffer), m_materialGlobals.m_pPixelConstantBufferData);
    Render::CreateConstantBuffer(device, sizeof(TerrainVertexConstantBuffer), m_materialGlobals.m_pTerrainVertexConstantBufferData);

    for (size_t i = 0; i < 4ull; i++)
    {
        size_t lodSteps = Chunk::Steps >> i;

        std::vector<TerrainVertex> vertices;
        vertices.reserve((lodSteps + 1) * (lodSteps + 1));

        float step = static_cast<float>(1 << i);
        for (int z = 0; z <= lodSteps; ++z)
        {
            for (int x = 0; x <= lodSteps; ++x)
            {
                vertices.push_back({ Math::Vector2(step * x, step * z) });
            }
        }
        std::vector<uint32_t> indices;
        indices.reserve(lodSteps * lodSteps * 6);

        auto index = [lodSteps](int x, int z)
        {
            return z * (lodSteps + 1) + x;
        };

        for (int z = 0; z < lodSteps; ++z)
        {
            for (int x = 0; x < lodSteps; ++x)
            {
                uint32_t i0 = index(x, z);
                uint32_t i1 = index(x + 1, z);
                uint32_t i2 = index(x, z + 1);
                uint32_t i3 = index(x + 1, z + 1);

                indices.push_back(i0);
                indices.push_back(i1);
                indices.push_back(i2);

                indices.push_back(i1);
                indices.push_back(i3);
                indices.push_back(i2);
            }
        }
        m_materialGlobals.m_lods[i].m_nbIndices = lodSteps * lodSteps * 6;


        {
            D3D11_BUFFER_DESC vbDesc = {};
            vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
            vbDesc.ByteWidth = UINT(vertices.size() * sizeof(TerrainVertex));
            vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

            D3D11_SUBRESOURCE_DATA vbData = {};
            vbData.pSysMem = vertices.data();

            device->CreateBuffer(&vbDesc, &vbData, m_materialGlobals.m_lods[i].m_pVertexBuffer.GetAddressOf());
        }

        {
            D3D11_BUFFER_DESC ibDesc = {};
            ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
            ibDesc.ByteWidth = UINT(indices.size() * sizeof(uint32_t));
            ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

            D3D11_SUBRESOURCE_DATA ibData = {};
            ibData.pSysMem = indices.data();

            device->CreateBuffer(&ibDesc, &ibData, m_materialGlobals.m_lods[i].m_pIndexBuffer.GetAddressOf());
        }
    }
}


bool TerrainMaterial::Load(const std::filesystem::path& file)
{
    return true;
}

void TerrainMaterial::Render(ID3D11DeviceContext* context, const DeviceResources& deviceResources, TerrainComponent* terrain)
{

    Render::SetShaders(m_materialGlobals.m_pPixelShader, m_materialGlobals.m_pVertexShader, m_materialGlobals.m_pInputLayout, context);
    context->RSSetState(deviceResources.GetBackfaceCullingRasterizer());

    m_materialGlobals.m_pixelConstantBuffer.RaycastHitPos.x = g_mouseRayHit.x;
    m_materialGlobals.m_pixelConstantBuffer.RaycastHitPos.y = g_mouseRayHit.y;
    m_materialGlobals.m_pixelConstantBuffer.RaycastHitPos.z = g_mouseRayHit.z;
    m_materialGlobals.m_pixelConstantBuffer.BrushSize = g_brushSize;
    m_materialGlobals.m_pixelConstantBuffer.WorldSize = terrain->m_terrainSize;

    m_materialGlobals.m_terrainVertexConstantBuffer.WorldMaxHeight = terrain->m_maxHight;
    m_materialGlobals.m_terrainVertexConstantBuffer.WorldSize = m_materialGlobals.m_pixelConstantBuffer.WorldSize;


    auto& materialData = m_materialGlobals.m_pixelConstantBuffer.MaterialData;


    //for (size_t i = 0; i < 4; i++)
    //{
    //
    //
    //    size_t albedoIndex = Albedo0 + i;
    //    size_t normalIndex = Normal0 + i;
    //    size_t roughnessIndex = Roughness0 + i;
    //    if (m_materials[i] == nullptr)
    //    {
    //        materialData.set(albedoIndex, false);
    //        materialData.set(normalIndex, false);
    //        materialData.set(roughnessIndex, false);
    //        continue;
    //    }
    //
    //    materialData.set(albedoIndex, m_materials[i]->GetTexture(DefaultMaterial::TextureType::BaseColor) != nullptr);
    //    materialData.set(normalIndex, m_materials[i]->GetTexture(DefaultMaterial::TextureType::Normal) != nullptr);
    //    materialData.set(roughnessIndex, m_materials[i]->GetTexture(DefaultMaterial::TextureType::Roughness) != nullptr);
    //
    //
    //
    //    auto SetShaderResource = [&](size_t shaderTextureBitIndex, int materialTextureIndex)
    //    {
    //        if (materialData.test(shaderTextureBitIndex))
    //            context->PSSetShaderResources(shaderTextureBitIndex, 1, m_materials[i]->GetTexture(materialTextureIndex)->GetResourceView().GetAddressOf());
    //    };
    //
    //
    //    SetShaderResource(albedoIndex, DefaultMaterial::TextureType::BaseColor);
    //    SetShaderResource(normalIndex, DefaultMaterial::TextureType::Normal);
    //    SetShaderResource(roughnessIndex, DefaultMaterial::TextureType::Roughness);
    //}
    if (m_materials[0])
    {
        context->PSSetShaderResources(1, 1, m_materials[0]->GetTexture(DefaultMaterial::TextureType::BaseColor)->GetResourceView().GetAddressOf());
    }
    if (m_materials[1])
    {
        context->PSSetShaderResources(2, 1, m_materials[1]->GetTexture(DefaultMaterial::TextureType::BaseColor)->GetResourceView().GetAddressOf());
    }
    if (m_materials[2])
    {
        context->PSSetShaderResources(3, 1, m_materials[2]->GetTexture(DefaultMaterial::TextureType::BaseColor)->GetResourceView().GetAddressOf());
    }
    if (m_materials[3])
    {
        context->PSSetShaderResources(4, 1, m_materials[3]->GetTexture(DefaultMaterial::TextureType::BaseColor)->GetResourceView().GetAddressOf());
    }

    if (terrain->m_worldTexture)
    {
        context->PSSetShaderResources(0, 1, terrain->m_worldTexture->GetResourceView().GetAddressOf());
        context->VSSetShaderResources(1, 1, terrain->m_worldTexture->GetResourceView().GetAddressOf());

    }
    Render::UpdateConstantBuffer(Render::SHADER_TYPE_PIXEL, 1, m_materialGlobals.m_pPixelConstantBufferData, &m_materialGlobals.m_pixelConstantBuffer, context);



    UINT strides[1] =
    {
        sizeof(TerrainVertex)
    };

    UINT offsets[1] = { 0 };
    ID3D11SamplerState* samplers[1] = {
        deviceResources.GetDefaultSamplerState()
    };
    context->VSSetSamplers(0, 1, samplers);



    auto camera = RenderManager::GetInstance()->GetCamera();


    for (const Chunk& chunk : terrain->m_chunks)
    {
        float x = chunk.SizeInMeter * chunk.m_x;
        float z = chunk.SizeInMeter * chunk.m_z;



        DirectX::BoundingBox boundingBox;
        DirectX::BoundingBox::CreateFromPoints(boundingBox, 
            Spectral::DxMathUtils::ToDx(Math::Vector3(x, chunk.m_minHeightBound * terrain->m_maxHight - (terrain->m_maxHight*0.5f), z)), 
            Spectral::DxMathUtils::ToDx(Math::Vector3(x + chunk.SizeInMeter, chunk.m_maxHeightBound * terrain->m_maxHight - (terrain->m_maxHight * 0.5f), z + chunk.SizeInMeter)
            )
        );
        float distanceToCamera = (Math::Vector3(boundingBox.Center.x, boundingBox.Center.y, boundingBox.Center.z) - camera->GetWorldMatrix().GetPosition()).Length();

        int lod = static_cast<int>(std::roundf(std::clamp(distanceToCamera / 500.0f,0.0f, 3.0f)));


        if (camera->m_frustum.Contains(boundingBox))
        {
            m_materialGlobals.m_terrainVertexConstantBuffer.StartX = static_cast<float>(chunk.m_x) * Chunk::Steps;
            m_materialGlobals.m_terrainVertexConstantBuffer.StartZ = static_cast<float>(chunk.m_z) * Chunk::Steps;
            Render::UpdateConstantBuffer(Render::SHADER_TYPE_VERTEX, 1, m_materialGlobals.m_pTerrainVertexConstantBufferData, &m_materialGlobals.m_terrainVertexConstantBuffer, context);
            if (chunk.m_SRV)
            {
                context->VSSetShaderResources(0, 1, chunk.m_SRV.GetAddressOf());
            }
            

            ID3D11Buffer* buffers[1] ={ m_materialGlobals.m_lods[lod].m_pVertexBuffer.Get() };
            context->IASetIndexBuffer(m_materialGlobals.m_lods[lod].m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            context->IASetVertexBuffers(0, 1, buffers, strides, offsets);
            context->DrawIndexed(m_materialGlobals.m_lods[lod].m_nbIndices, 0 ,0);
        }

    }
}
