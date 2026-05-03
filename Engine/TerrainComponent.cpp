#include "TerrainComponent.h"

#include "iRender.h"
#include "Mesh.h"
#include "Editor.h"
#include "GameObject.h"
#ifdef EDITOR
#include "PropertyWindowFactory.h"
#endif // DEBUG
#include "IOManager.h"
#include "Texture.h"
#include "TerrainMaterial.h"
#include "ResourceManager.h"
#include "DefaultMaterial.h"
#include <random>
#include "GrassRenderer.h"
#include "Intersection.h"
#include <DirectXCollision.h>
#include "DxMathUtils.h"
#include "PerspectiveCamera.h"

#ifdef EDITOR
#include <MaterialEditor.h>
#endif // EDITOR
#include <Thumbnail.h>
#include "RenderManager.h"

TerrainComponent::TerrainComponent(GameObject* owner)
	: Component(owner)
    , m_terrainSize(500.0f)
{
    m_material = std::make_shared<TerrainMaterial>();

    //CreatePlaneMesh();

}

void TerrainComponent::Render()
{
    DirectX::BoundingBox boundingBox;



    auto camera = static_cast<PerspectiveCamera*>(Render::GetCamera());

    DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(
        camera->m_fov,
        camera->m_aspectRatio,
        camera->m_nearClip,
        camera->m_farClip
    );

    DirectX::BoundingFrustum frustum;
    DirectX::BoundingFrustum::CreateFromMatrix(frustum, projMatrix);

    DirectX::XMMATRIX worldMatrix = Spectral::DxMathUtils::ToDx(camera->GetWorldMatrix());

    DirectX::BoundingFrustum worldFrustum;
    frustum.Transform(worldFrustum, worldMatrix);

    RenderManager::GetInstance()->GetInstanceManager()->AddInstance(this);

    for (size_t i = 0; i < m_grassPatches.size(); i++)
    {
        //DirectX::BoundingBox::CreateFromPoints(boundingBox, Spectral::DxMathUtils::ToDx(m_grassPatches[i].BoundingMin), Spectral::DxMathUtils::ToDx(m_grassPatches[i].BoundingMax));
        //if (worldFrustum.Intersects(boundingBox) || worldFrustum.Contains(boundingBox))
        {
            GrassRenderer::AddGrassPatch(m_grassPatches[i]);
        }
    }
}

void TerrainComponent::Update(float deltaTime)
{
}
#ifdef EDITOR

void TerrainComponent::ComponentEditor()
{


    {
        if (m_worldTexture && m_worldTexture->GetResourceView().Get())
        {
            auto selectedTextureName = m_worldTexture->GetFilename();
            ImGui::Text("World Texture");
            auto resource = m_worldTexture->GetResourceView().Get();
            if (ImGui::ImageButton(selectedTextureName.c_str(), resource, Editor::GetInstance()->GetDefaultTextureSize()))
            {
                PropertyWindowFactory::SelectTexture(m_worldTexture, selectedTextureName);

            }
        }
        else
        {
            ImGui::Text(std::string("World Texture").c_str());

            if (ImGui::Button(std::string("##selwtex").c_str(), Editor::GetInstance()->GetDefaultTextureSize()))
            {
                PropertyWindowFactory::SelectTexture(m_worldTexture);
            }
        }
    }
    ImGui::InputFloat("Height", &m_maxHight);

    if (!m_created)
    {
        ImGui::InputFloat("Size", &m_terrainSize);
        ImGui::InputInt("Resolution", &m_resolution);
        {
            ImGui::Text(std::string("Height Texture").c_str());

            if (m_heightTexture && m_heightTexture->GetResourceView().Get())
            {
                if (ImGui::ImageButton(m_heightTexture->GetFilename().c_str(), m_heightTexture->GetResourceView().Get(), Editor::GetInstance()->GetDefaultTextureSize()))
                {
                    PropertyWindowFactory::SelectTexture(m_heightTexture, m_heightTexture->GetFilename());
                }
            }
            else
            {
                if (ImGui::Button(std::string("##selhtex").c_str(), Editor::GetInstance()->GetDefaultTextureSize()))
                {
                    PropertyWindowFactory::SelectTexture(m_heightTexture);
                }
            }
            ImGui::InputFloat("BlurRadius", &m_terrainGenerationBlurRadius);
            ImGui::InputFloat("BlurSigma", &m_terrainGenerationBlurSigma);
        }

        if (ImGui::Button("Create Terrain") && m_terrainSize > 10)
        {
            if (m_heightTexture)
            {
                CreateTerrain();
                m_created = true;
            }
        }



        return;
    }

    if (ImGui::Button("Regenerate Terrain"))
    {
        m_chunks.clear();
        m_created = false;
    }

    if (ImGui::Button("Open Terrain Editor"))
    {
        Editor::GetInstance()->OpenTerrainEditor(this);
    }

    static std::vector<std::pair<std::string, int>> materials{
        {"Material0", 0},
        {"Material1", 1},
        {"Material2", 2},
        {"Material3", 3},
    };
    static std::shared_ptr<Thumbnail> materialThumbnails[4];
    for (const auto& [textureName, materialId] : materials)
    {
        
        if (m_material->m_materials[materialId])
        {
            auto selectedMaterialName = m_material->m_materials[materialId]->GetFilename();
            ImGui::Text(std::string(textureName + ": " + selectedMaterialName).c_str());
            //if (ImGui::Button(std::string("##" + textureName).c_str(), Editor::GetInstance()->GetDefaultTextureSize()))
            //{
            //}

            ImGui::Image(ThumbnailManager::GetThumbnail(m_material->m_materials[materialId].get())->GetSRV(), Editor::GetInstance()->GetDefaultTextureSize());
        }
        else
        {
            ImGui::Text(std::string(textureName + ": Not selected").c_str());

            if (ImGui::Button(std::string("##" + textureName).c_str(), Editor::GetInstance()->GetDefaultTextureSize()))
            {
            }
        }
        if (Editor::GetInstance()->GetDropResource(m_material->m_materials[materialId]))
        {
        }

        if (m_material->m_materials[materialId] != nullptr)
        {

            if (ImGui::CollapsingHeader(m_material->m_materials[materialId]->m_filename.c_str()))
            {
                MaterialEditor::RenderGUI(m_material->m_materials[materialId]);
            }
        }

        ImGui::Separator();
    }

}
void TerrainComponent::DisplayComponentIcon()
{
    ImGui::SameLine();
    ImGui::Image(ResourceManager::GetInstance()->GetResource<Texture>("Terrain.bmp")->GetResourceView().Get(), ImVec2(15, 15));
}
#endif

Json::Object TerrainComponent::SaveComponent()
{
    Json::Object object;

    object.emplace("Terrain Name", GetOwner()->GetName());

    object.emplace("WorldColorTexture", m_worldTexture ? m_worldTexture->GetFilename() : "null");
    object.emplace("MaxHeight", m_maxHight);

    object.emplace("Material0", m_material->m_materials[0] ? m_material->m_materials[0]->GetFilename() : "null");
    object.emplace("Material1", m_material->m_materials[1] ? m_material->m_materials[1]->GetFilename() : "null");
    object.emplace("Material2", m_material->m_materials[2] ? m_material->m_materials[2]->GetFilename() : "null");
    object.emplace("Material3", m_material->m_materials[3] ? m_material->m_materials[3]->GetFilename() : "null");


    //IOManager::SaveSpectralModel(GetMesh());

    
    return std::move(object);
}

void TerrainComponent::LoadComponent(const rapidjson::Value& object)
{

    std::filesystem::path p = IOManager::ProjectDirectory / IOManager::GetResourceData<ResourceType::Model>().Folder / "UserTerrain.terrain";

    if (std::filesystem::exists(p))
    {
        m_created = true;

        ReadObject in(IOManager::ProjectDirectory / IOManager::GetResourceData<ResourceType::Model>().Folder / "UserTerrain.terrain");
        uint32_t version;

        in.Read(version);
        if (version == 2)
        {
            in.Read(m_terrainSize);
            in.Read(m_resolution);

            size_t nbChunks = 0;
            in.Read(nbChunks);

            for (size_t i = 0; i < nbChunks; i++)
            {

                auto& chunk = m_chunks.emplace_back(Chunk());
                in.Read(chunk.m_x);
                in.Read(chunk.m_z);
                in.Read(chunk.m_heights);

                chunk.Build();
            }
        }
    }

    if (object.HasMember("WorldColorTexture") && strcmp(object["WorldColorTexture"].GetString(), "null") != 0)
    {
        m_worldTexture = ResourceManager::GetInstance()->GetResource<Texture>(object["WorldColorTexture"].GetString());
    }

    if (object.HasMember("MaxHeight"))
    {
        m_maxHight = object["MaxHeight"].GetFloat();
    }

    if (object.HasMember("Material0") && strcmp(object["Material0"].GetString(), "null") != 0)
    {
        m_material->m_materials[0] = ResourceManager::GetInstance()->GetResource<DefaultMaterial>(object["Material0"].GetString());
    }
    if (object.HasMember("Material1") && strcmp(object["Material1"].GetString(), "null") != 0)
    {
        m_material->m_materials[1] = ResourceManager::GetInstance()->GetResource<DefaultMaterial>(object["Material1"].GetString());
    }
    if (object.HasMember("Material2") && strcmp(object["Material2"].GetString(), "null") != 0)
    {
        m_material->m_materials[2] = ResourceManager::GetInstance()->GetResource<DefaultMaterial>(object["Material2"].GetString());
    }
    if (object.HasMember("Material3") && strcmp(object["Material3"].GetString(), "null") != 0)
    {
        m_material->m_materials[3] = ResourceManager::GetInstance()->GetResource<DefaultMaterial>(object["Material3"].GetString());
    }
}


void TerrainComponent::GenerateGrass(GrassPatch& patch)
{
    if (patch.GrassPositionBufferData != nullptr)
        patch.GrassPositionBufferData.Reset();

    auto GetPosTerrainSurfaceHeight = [&](float x, float z)
    {
        //float fx = x / m_terrainSize * m_vertexRowCount;
        //float fz = z / m_terrainSize * m_vertexRowCount;
        //
        //int ix = static_cast<int>(floor(fx));
        //int iz = static_cast<int>(floor(fz));
        //
        //float fracX = fx - ix;
        //float fracZ = fz - iz;
        //
        //ix = std::clamp(ix, 0, static_cast<int>(m_vertexRowCount) - 1);
        //iz = std::clamp(iz, 0, static_cast<int>(m_vertexRowCount) - 1);
        //
        //const int rowStride = m_vertexRowCount + 1;
        //
        //float h00 = m_vertices[iz * rowStride + ix].Height;
        //float h10 = m_vertices[iz * rowStride + (ix + 1)].Height;
        //float h01 = m_vertices[(iz + 1) * rowStride + ix].Height;
        //float h11 = m_vertices[(iz + 1) * rowStride + (ix + 1)].Height;
        //
        //float height =
        //    (1.0f - fracX) * (1.0f - fracZ) * h00 +
        //    fracX * (1.0f - fracZ) * h10 +
        //    (1.0f - fracX) * fracZ * h01 +
        //    fracX * fracZ * h11;

        return 0.0f;
    };

    float tuftSize = 1.0f;
    int grassBladesPerTuft = 30;

    Math::Vector3 pos = m_owner->GetPosition();


    std::vector<GrassVertexV1> bladePositions(patch.GrassTufts.size() * grassBladesPerTuft);

    std::size_t seed = std::hash<float>()(patch.BoundingMin.x)
        ^ (std::hash<float>()(patch.BoundingMin.y) << 1)
        ^ (std::hash<float>()(patch.BoundingMin.z) << 2);

    std::mt19937 gen(static_cast<uint32_t>(seed));
    std::uniform_real_distribution<float> dist(-tuftSize, tuftSize);


    for (size_t tuftI = 0; tuftI < patch.GrassTufts.size(); tuftI++)
    {
        Math::Vector3 tuftPosition = patch.GrassTufts[tuftI];

        for (size_t bladeI = 0; bladeI < grassBladesPerTuft; bladeI++)
        {
            auto& [position, direction] = bladePositions[tuftI * grassBladesPerTuft + bladeI];
         
            position.x = dist(gen) + tuftPosition.x;
            position.z = dist(gen) + tuftPosition.z;
            position.y = GetPosTerrainSurfaceHeight(position.x, position.z);


            Math::Vector3 dirToTuft = (
                Math::Vector3(position.x, 0.0f, position.z) -
                Math::Vector3(tuftPosition.x,0.0f, tuftPosition.z)
                );

            dirToTuft.Normalize();

            position += pos;
            direction.x = dirToTuft.x;
            direction.y = dirToTuft.z;
        }
    }

    if (!bladePositions.empty())
    {
        patch.GrassPositionBufferData = Render::CreateVertexBuffer(Render::GetDevice(), bladePositions);
    }
    patch.NumGrassPositions = static_cast<unsigned int>(bladePositions.size());
}
void GaussianBlur(
    const std::vector<float>& input,
    std::vector<float>& result,
    int width,
    int height,
    int radius,
    float sigma)
{
    // --- Build kernel ---
    int size = radius * 2 + 1;
    std::vector<float> kernel(size);

    float sum = 0.0f;
    for (int i = 0; i < size; ++i)
    {
        int x = i - radius;
        float v = std::exp(-(x * x) / (2.0f * sigma * sigma));
        kernel[i] = v;
        sum += v;
    }

    for (float& v : kernel)
        v /= sum;

    // --- Temp buffer for separable pass ---
    std::vector<float> temp(width * height);

    // --- Horizontal pass ---
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float accum = 0.0f;

            for (int k = -radius; k <= radius; ++k)
            {
                int sx = std::clamp(x + k, 0, width - 1);
                accum += input[y * width + sx] * kernel[k + radius];
            }

            temp[y * width + x] = accum;
        }
    }

    // --- Vertical pass ---
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float accum = 0.0f;

            for (int k = -radius; k <= radius; ++k)
            {
                int sy = std::clamp(y + k, 0, height - 1);
                accum += temp[sy * width + x] * kernel[k + radius];
            }

            result[y * width + x] = accum;
        }
    }
}
void TerrainComponent::CreateTerrain()
{

    //Math::Vector3 pos = m_owner->GetPosition();
    //int b = static_cast<int>(m_terrainSize / GrassPatch::PatchSize);
    //for (int x = 0; x < b; x++)
    //{
    //    for (int z = 0; z < b; z++)
    //    {
    //        GrassPatch patch{};
    //        patch.BoundingMin = Math::Vector3(static_cast<float>(x) * GrassPatch::PatchSize, 0.0f, static_cast<float>(z) * GrassPatch::PatchSize) + pos;
    //        patch.BoundingMax = patch.BoundingMin + Math::Vector3(GrassPatch::PatchSize, 100.0f, GrassPatch::PatchSize) + pos;
    //        m_grassPatches.push_back(patch);
    //    }
    //}


    //D3D11_TEXTURE2D_DESC desc;
    //m_heightTexture->GetTexture()->GetDesc(&desc);
    //
    //D3D11_TEXTURE2D_DESC stagingDesc = desc;
    //stagingDesc.Usage = D3D11_USAGE_STAGING;
    //stagingDesc.BindFlags = 0;
    //stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    //stagingDesc.MiscFlags = 0;
    //
    //ID3D11Texture2D* staging = nullptr;
    //Render::GetDevice()->CreateTexture2D(&stagingDesc, nullptr, &staging);
    //
    //Render::GetContext().GetContext()->CopyResource(staging, m_heightTexture->GetTexture().Get());
    //
    //D3D11_MAPPED_SUBRESOURCE mapped;
    //Render::GetContext().GetContext()->Map(staging, 0, D3D11_MAP_READ, 0, &mapped);
    //
    //std::vector<uint8_t> buffer(desc.Height * mapped.RowPitch);
    //memcpy(buffer.data(), mapped.pData, buffer.size());
    //
    //Render::GetContext().GetContext()->Unmap(staging, 0);
    //staging->Release();
    D3D11_TEXTURE2D_DESC desc;
    m_heightTexture->GetTexture()->GetDesc(&desc);

    if (desc.Format != DXGI_FORMAT_R32G32B32_FLOAT) {
        // Wrong format
        return;
    }

    // --- Create staging texture ---
    D3D11_TEXTURE2D_DESC stagingDesc = desc;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.MiscFlags = 0;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> staging;
    HRESULT hr = Render::GetDevice()->CreateTexture2D(&stagingDesc, nullptr, &staging);
    if (FAILED(hr)) return;

    // --- Copy GPU → CPU ---
    Render::GetContext().GetContext()->CopyResource(staging.Get(), m_heightTexture->GetTexture().Get());

    // (optional debug sync)
    // context->Flush();

    // --- Map ---
    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = Render::GetContext().GetContext()->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr)) return;

    // --- Allocate output (tightly packed RGB floats) ---
    std::vector<float> result(desc.Width * desc.Height);

    for (UINT y = 0; y < desc.Height; ++y)
    {
        float* srcRow = (float*)((uint8_t*)mapped.pData + y * mapped.RowPitch);
        float* dstRow = result.data() + y * desc.Width;

        for (UINT x = 0; x < desc.Width; ++x)
        {
            dstRow[x] = srcRow[x * 3 + 0]; // R
        }
    }

    
    std::vector<float> blurred(desc.Width * desc.Height);
    GaussianBlur(result, blurred, desc.Width, desc.Height, m_terrainGenerationBlurRadius, m_terrainGenerationBlurSigma);

    Render::GetContext().GetContext()->Unmap(staging.Get(), 0);



    size_t width = static_cast<size_t>(m_terrainSize / Chunk::SizeInMeter);
    m_terrainSize = width * Chunk::Steps;
    m_chunks.reserve(width * width);
    Math::Vector2 textureSize(static_cast<float>(desc.Width), static_cast<float>(desc.Height));
    for (size_t x = 0; x < width; x++)
    {
        for (size_t z = 0; z < width; z++)
        {
            m_chunks.emplace_back(Chunk(static_cast<uint16_t>(x), static_cast<uint16_t>(z), blurred, textureSize, m_terrainSize));

            Math::Vector2 pos((float)x * Chunk::Steps, (float)z * Chunk::Steps);
            m_instances.push_back(pos);

        }
    }

    {
        D3D11_BUFFER_DESC ibDesc = {};
        ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
        ibDesc.ByteWidth = UINT(sizeof(Math::Vector2) * m_instances.size());
        ibDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA ibData = {};
        ibData.pSysMem = m_instances.data();

        Render::GetDevice()->CreateBuffer(&ibDesc, &ibData, m_pInstanceBuffer.GetAddressOf());
    }
    m_material->m_materials[0] = ResourceManager::GetInstance()->GetResource<DefaultMaterial>("Default.material");
    for (Chunk& chunk : m_chunks)
    {
        chunk.Build();
    }

    auto path = IOManager::ProjectDirectory / IOManager::GetResourceData<ResourceType::Model>().Folder / "UserTerrain.terrain";
    WriteObject out(path);

    uint32_t version = 2;

    out.Write(version);
    out.Write(m_terrainSize);
    out.Write(m_resolution);
    out.Write(m_chunks.size());
    for (const Chunk& chunk : m_chunks)
    {
        out.Write(chunk.m_x);
        out.Write(chunk.m_z);
        out.Write(chunk.m_heights);
    }


    out.Write(m_grassPatches.size());
    for (const GrassPatch& patch : m_grassPatches)
    {
        out.Write(patch.GrassTufts);
    }
}

float TerrainComponent::GetTerrainSize() const
{
    return m_terrainSize;
}

Chunk::Chunk(uint16_t x, uint16_t z,
    const std::vector<float>& heightField,
    Math::Vector2 heightFieldTextureSize,
    size_t maxTerrainSize)
{

    UINT steps = Steps + 1;

    m_heights.resize(steps * steps);

    size_t width = static_cast<size_t>(heightFieldTextureSize.x);
    size_t height = static_cast<size_t>(heightFieldTextureSize.y);

    for (size_t dx = 0; dx < steps; dx++)
    {
        for (size_t dz = 0; dz < steps; dz++)
        {
            size_t originX = x * Steps + dx;
            size_t originZ = z * Steps + dz;

            float u = static_cast<float>(originX) / static_cast<float>(maxTerrainSize);
            float v = static_cast<float>(originZ) / static_cast<float>(maxTerrainSize);

            // Scale to texture space
            float fx = u * (width - 1);
            float fy = v * (height - 1);

            // Integer coords
            size_t x0 = static_cast<size_t>(fx);
            size_t y0 = static_cast<size_t>(fy);

            size_t x1 = std::min(x0 + 1, width - 1);
            size_t y1 = std::min(y0 + 1, height - 1);

            // Fractional part
            float tx = fx - static_cast<float>(x0);
            float ty = fy - static_cast<float>(y0);

            // Sample 4 neighbors
            float h00 = heightField[y0 * width + x0];
            float h10 = heightField[y0 * width + x1];
            float h01 = heightField[y1 * width + x0];
            float h11 = heightField[y1 * width + x1];

            // Interpolate
            float hx0 = h00 + (h10 - h00) * tx;
            float hx1 = h01 + (h11 - h01) * tx;
            float h = hx0 + (hx1 - hx0) * ty;

            m_heights[dz * steps + dx] = h;
        }
    }


    m_x = x;
    m_z = z;

}

void Chunk::Build()
{
    m_minHeightBound =  FLT_MAX;
    m_maxHeightBound = -FLT_MAX;

    for (float height : m_heights)
    {
        m_minHeightBound = std::min(m_minHeightBound, height);
        m_maxHeightBound = std::max(m_maxHeightBound, height);
    }



    UINT steps = Steps + 1;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = steps;
    desc.Height = steps;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R32_FLOAT;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;


    D3D11_SUBRESOURCE_DATA subresource = {};
    subresource.pSysMem = m_heights.data();
    subresource.SysMemPitch = steps * sizeof(float);
    subresource.SysMemSlicePitch = 0;

    Render::GetDevice()->CreateTexture2D(&desc,&subresource,m_texture.GetAddressOf());
    Render::GetDevice()->CreateShaderResourceView(m_texture.Get(), nullptr, m_SRV.GetAddressOf());
    Render::GetDevice()->CreateUnorderedAccessView(m_texture.Get(), nullptr, m_UAV.GetAddressOf());
}
