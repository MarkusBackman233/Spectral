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
#include <execution>
#include "PhysXManager.h"
#include "MathFunctions.h"
#include "Model.h"


TerrainComponent::TerrainComponent(GameObject* owner)
	: Component(owner)
    , m_terrainSize(500.0f)
{
    m_material = std::make_shared<TerrainMaterial>();
    m_actor = PhysXManager::GetInstance()->CreateActor(PhysXManager::PhysicsType::StaticActor, m_owner->GetWorldMatrix());


    //CreatePlaneMesh();

}

TerrainComponent::~TerrainComponent()
{
    if (m_actor)
    {
        m_actor->release();
        m_actor = nullptr;
    }
}

void TerrainComponent::Render()
{
    DirectX::BoundingBox boundingBox;
    RenderManager::GetInstance()->GetInstanceManager()->AddInstance(this);
    auto camera = RenderManager::GetInstance()->GetCamera();



    for (const Chunk& chunk : m_chunks)
    {
        float x = chunk.SizeInMeter * chunk.m_x;
        float z = chunk.SizeInMeter * chunk.m_z;



        DirectX::BoundingBox boundingBox;
        DirectX::BoundingBox::CreateFromPoints(boundingBox,
            Spectral::DxMathUtils::ToDx(Math::Vector3(x, chunk.m_minHeightBound, z)),
            Spectral::DxMathUtils::ToDx(Math::Vector3(x + chunk.SizeInMeter, chunk.m_maxHeightBound, z + chunk.SizeInMeter)
            )
        );
        float distanceToCamera = (Math::Vector3(boundingBox.Center.x, boundingBox.Center.y, boundingBox.Center.z) - camera->GetWorldMatrix().GetPosition()).Length();

        int lod = static_cast<int>(std::roundf(std::clamp(distanceToCamera / 500.0f, 0.0f, 3.0f)));


        if (camera->m_frustum.Contains(boundingBox))
        {
            for (auto& [model, instance] : chunk.m_clutterInstances)
            {
                if (lod == 0)
                {
                    RenderManager::GetInstance()->GetInstanceManager()->AddInstance(DrawableInstance{ model->m_root.m_mesh, model->GetMaterials()[0] }, instance.m_instanceData);
                }


                if (!model->m_billboardVertices.empty())
                {
                    BillboardRenderer::AddBillboard(model->GetMaterials()[1], BillboardRenderData{ model->m_billboardBuffer, instance.m_instanceData });
                }
            }
        }
    }



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

    if (!m_created)
    {
        ImGui::InputFloat("Size", &m_terrainSize);
        ImGui::InputInt("Resolution", &m_resolution);
        ImGui::InputFloat("Height", &m_maxHight);

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
    PxHeightFieldDesc hfDesc;
    hfDesc.format = PxHeightFieldFormat::eS16_TM;
    hfDesc.nbColumns = 128;
    hfDesc.nbRows = 128;
    hfDesc.samples.stride = sizeof(PxHeightFieldSample);






    std::filesystem::path p = IOManager::ProjectDirectory / IOManager::GetResourceData<ResourceType::Model>().Folder / "UserTerrain.terrain";

    if (std::filesystem::exists(p))
    {
        m_created = true;

        ReadObject in(IOManager::ProjectDirectory / IOManager::GetResourceData<ResourceType::Model>().Folder / "UserTerrain.terrain");
        uint32_t version;

        in.Read(version);
        if (version == 3)
        {
            in.Read(m_terrainSize);
            in.Read(m_resolution);
            in.Read(m_terrainGenerationBlurRadius);
            in.Read(m_terrainGenerationBlurSigma);

            size_t nbChunks = 0;
            in.Read(nbChunks);

            for (size_t i = 0; i < nbChunks; i++)
            {

                auto& chunk = m_chunks.emplace_back(Chunk());
                in.Read(chunk.m_x);
                in.Read(chunk.m_z);
                in.Read(chunk.m_heights);
                in.Read(chunk.m_normals);
                chunk.Build();


                std::vector<PxHeightFieldSample> hs;
                hs.reserve((Chunk::Steps+1) * (Chunk::Steps + 1));

                float totalHeight = chunk.m_maxHeightBound - chunk.m_minHeightBound;

                int size = Chunk::Steps + 1;
                for (int x = 0; x < hfDesc.nbColumns; x++)
                {
                    for (int z = 0; z < hfDesc.nbRows; z++)
                    {
                            PxHeightFieldSample s{};
                            float u = float(x) / float(hfDesc.nbColumns - 1);
                            float v = float(z) / float(hfDesc.nbRows - 1);

                            u = 1.0f - u;

                            int stepX = int(u * (size - 1));
                            int stepZ = int(v * (size - 1));

                            float normalized = (chunk.m_heights[int(stepX) * size + int(stepZ)] - chunk.m_minHeightBound) / totalHeight;
                            PxI16 physxHeight = PxI16(normalized * 32767);
                            s.height = static_cast<PxI16>(physxHeight);

                            s.materialIndex0 = 0;
                            s.materialIndex1 = 0;
                            s.clearTessFlag();

                            hs.push_back(s);
                    }
                }

                hfDesc.samples.data = hs.data();
                PxHeightField* heightField = PxCreateHeightField(hfDesc, PhysXManager::GetInstance()->GetPhysics()->getPhysicsInsertionCallback());
                float heightScale = totalHeight / 32767.0f;
                float scale = 128.0f / (hfDesc.nbColumns - 1);
                PxHeightFieldGeometry hfGeom(
                    heightField,
                    PxMeshGeometryFlags(),
                    heightScale, // heightScale
                    scale,
                    scale
                );


                PxShape* hfShape = PxRigidActorExt::createExclusiveShape(
                    *m_actor,
                    hfGeom,
                    *PhysXManager::GetInstance()->GetDefaultMaterial()
                );


                PxQuat q(Math::ConvertToRadians(90.0f),PxVec3(0.0f,1.0f,0.0f));


                PxTransform t(PxVec3(128.0f * chunk.m_x, chunk.m_minHeightBound, 128.0f * chunk.m_z + 128.0f),q);

                hfShape->setLocalPose(t);

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
            //position.y = GetPosTerrainSurfaceHeight(position.x, position.z);


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
void TerrainComponent::CreateTerrain()
{
    size_t width = static_cast<size_t>(m_terrainSize / Chunk::SizeInMeter);
    m_terrainSize = width * Chunk::Steps;

    HeightmapProcessor::Settings settings;
    settings.blurRadius = m_terrainGenerationBlurRadius;
    settings.blurSigma = m_terrainGenerationBlurSigma;
    settings.useGaussianBlur = true;
    HeightmapProcessor hp(Render::GetDevice(),Render::GetContext().GetContext(), m_heightTexture->GetTexture().Get(), m_terrainSize, settings);
    settings.useGaussianBlur = false;
    HeightmapProcessor hp2(Render::GetDevice(),Render::GetContext().GetContext(), m_worldTexture->GetTexture().Get(), m_terrainSize, settings);


    m_chunks.resize(width * width);

    std::vector<size_t> indices(width* width);
    for (size_t i = 0; i < indices.size(); ++i)
        indices[i] = i;

    std::for_each(std::execution::par, indices.begin(), indices.end(),
        [&](size_t i)
    {
        size_t x = i % width;
        size_t z = i / width;

        m_chunks[i].Feed(
            static_cast<uint16_t>(x),
            static_cast<uint16_t>(z),
            hp, hp2, m_maxHight);
    });

    {
        D3D11_BUFFER_DESC ibDesc = {};
        ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
        ibDesc.ByteWidth = UINT(sizeof(Math::Vector2) * m_instances.size());
        ibDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA ibData = {};
        ibData.pSysMem = m_instances.data();

        Render::GetDevice()->CreateBuffer(&ibDesc, &ibData, m_pInstanceBuffer.GetAddressOf());
    }
    for (Chunk& chunk : m_chunks)
    {
        chunk.Build();
    }

    auto path = IOManager::ProjectDirectory / IOManager::GetResourceData<ResourceType::Model>().Folder / "UserTerrain.terrain";
    WriteObject out(path);

    uint32_t version = 3;

    out.Write(version);
    out.Write(m_terrainSize);
    out.Write(m_resolution);
    out.Write(m_terrainGenerationBlurRadius);
    out.Write(m_terrainGenerationBlurSigma);
    out.Write(m_chunks.size());
    for (const Chunk& chunk : m_chunks)
    {
        out.Write(chunk.m_x);
        out.Write(chunk.m_z);
        out.Write(chunk.m_heights);
        out.Write(chunk.m_normals);
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

float TerrainComponent::GetHeightAtPosition(const Math::Vector3& Position) const
{
    int xChunk = static_cast<int>(Position.x / Chunk::SizeInMeter);
    int zChunk = static_cast<int>(Position.z / Chunk::SizeInMeter);

    size_t width = static_cast<size_t>(m_terrainSize / Chunk::SizeInMeter);

    if (xChunk < 0 || zChunk < 0 || xChunk >= width || zChunk >= width)
        return 0.0f;

    const Chunk& chunk = m_chunks[zChunk * width + xChunk];

    float localX = Position.x - (xChunk * Chunk::SizeInMeter);
    float localZ = Position.z - (zChunk * Chunk::SizeInMeter);

    float stepSize = Chunk::SizeInMeter / (Chunk::Steps );

    int xHeight = static_cast<int>(localX / stepSize);
    int zHeight = static_cast<int>(localZ / stepSize);

    xHeight = std::clamp(xHeight, 0, static_cast<int>(Chunk::Steps));
    zHeight = std::clamp(zHeight, 0, static_cast<int>(Chunk::Steps));

    return chunk.m_heights[zHeight * (Chunk::Steps+1) + xHeight];
}

physx::PxRigidActor* TerrainComponent::GetActor()
{
    return m_actor;
}

uint32_t PackNormal(const Math::Vector3& n)
{
    uint8_t r = (uint8_t)((n.x * 0.5f + 0.5f) * 255.0f);
    uint8_t g = (uint8_t)((n.y * 0.5f + 0.5f) * 255.0f);
    uint8_t b = (uint8_t)((n.z * 0.5f + 0.5f) * 255.0f);
    uint8_t a = 255;

    return (a << 24) | (b << 16) | (g << 8) | r;
}


void Chunk::Feed(uint16_t x, uint16_t z, const HeightmapProcessor& heightField, const HeightmapProcessor& heightField2, float maxTerrainHeight)
{


    UINT steps = Steps + 1;

    m_heights.resize(steps * steps);
    m_normals.resize(steps * steps);

    for (size_t dx = 0; dx < steps; dx++)
    {
        for (size_t dz = 0; dz < steps; dz++)
        {
            size_t originX = x * Steps + dx;
            size_t originZ = z * Steps + dz;

            auto GetCombinedHeight = [&](int x, int z)
            {
                return heightField.GetHeight(x, z) * maxTerrainHeight + heightField2.GetHeight(x, z) * 10.0f;
            };

            m_heights[dz * steps + dx] = GetCombinedHeight(originX, originZ);

            float hL = GetCombinedHeight(originX > 0 ? originX - 1 : originX, originZ);
            float hR = GetCombinedHeight(originX + 1, originZ);
            float hD = GetCombinedHeight(originX, originZ > 0 ? originZ - 1 : originZ);
            float hU = GetCombinedHeight(originX, originZ + 1);

            float scale = 1.0f;

            // Build tangent vectors
            Math::Vector3 dxVec(2.0f * scale, hR - hL, 0.0f);
            Math::Vector3 dzVec(0.0f, hU - hD, 2.0f * scale);

            // Cross to get normal
            Math::Vector3 normal = dzVec.Cross(dxVec).GetNormal();
            m_normals[dz * steps + dx] = PackNormal(normal);
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
    {
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

        Render::GetDevice()->CreateTexture2D(&desc,&subresource, m_heightTexture.GetAddressOf());
        Render::GetDevice()->CreateShaderResourceView(m_heightTexture.Get(), nullptr, m_heightSRV.GetAddressOf());
        Render::GetDevice()->CreateUnorderedAccessView(m_heightTexture.Get(), nullptr, m_heightUAV.GetAddressOf());
    }
    {
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = steps;
        desc.Height = steps;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA data = {};
        data.pSysMem = m_normals.data();
        data.SysMemPitch = steps * sizeof(uint32_t);

        Render::GetDevice()->CreateTexture2D(&desc, &data, m_normalTexture.GetAddressOf());
        Render::GetDevice()->CreateShaderResourceView(m_normalTexture.Get(), nullptr, m_normalSRV.GetAddressOf());
        Render::GetDevice()->CreateUnorderedAccessView(m_normalTexture.Get(), nullptr, m_normalUAV.GetAddressOf());
    }
}

float HeightmapProcessor::GetHeight(size_t x, size_t z) const
{
    float u = static_cast<float>(x) / m_maxTerrainSize;
    float v = static_cast<float>(z) / m_maxTerrainSize;

    // Scale to texture space
    float fx = u * (m_width - 1);
    float fy = v * (m_height - 1);

    // Integer coords
    size_t x0 = static_cast<size_t>(fx);
    size_t y0 = static_cast<size_t>(fy);

    size_t x1 = std::min(x0 + 1, static_cast<size_t>(m_width) - 1);
    size_t y1 = std::min(y0 + 1, static_cast<size_t>(m_height) - 1);

    // Fractional part
    float tx = fx - static_cast<float>(x0);
    float ty = fy - static_cast<float>(y0);

    // Sample 4 neighbors
    float h00 = m_heights[y0 * m_width + x0];
    float h10 = m_heights[y0 * m_width + x1];
    float h01 = m_heights[y1 * m_width + x0];
    float h11 = m_heights[y1 * m_width + x1];

    // Interpolate
    float hx0 = h00 + (h10 - h00) * tx;
    float hx1 = h01 + (h11 - h01) * tx;
    return hx0 + (hx1 - hx0) * ty;
}

HeightmapProcessor::HeightmapProcessor(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Texture2D* source, size_t maxTerrainSize, const Settings& settings)
    : m_maxTerrainSize(static_cast<float>(maxTerrainSize))
{

    if (!device || !context || !source)
        return;

    D3D11_TEXTURE2D_DESC desc;
    source->GetDesc(&desc);

    // --- Create staging texture ---
    D3D11_TEXTURE2D_DESC stagingDesc = desc;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.MiscFlags = 0;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> staging;

    HRESULT hr = device->CreateTexture2D(&stagingDesc, nullptr, &staging);
    if (FAILED(hr)) return ;

    // --- Copy GPU → CPU ---
    context->CopyResource(staging.Get(), source);

    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr)) return ;

    m_width = desc.Width;
    m_height = desc.Height;

    m_heights.resize(desc.Width * desc.Height);

    // --- Read pixels ---
    for (UINT y = 0; y < desc.Height; ++y)
    {
        uint8_t* row = (uint8_t*)mapped.pData + y * mapped.RowPitch;
        float* dst = m_heights.data() + y * desc.Width;

        for (UINT x = 0; x < desc.Width; ++x)
        {
            dst[x] = ReadHeightPixel(row, x, desc.Format, settings);
        }
    }

    context->Unmap(staging.Get(), 0);

    if (settings.useGaussianBlur)
    {
        std::vector<float> temp;
        GaussianBlur(m_heights, temp, desc.Width, desc.Height,settings.blurRadius, settings.blurSigma);
        m_heights.swap(temp);
    }
}

float HeightmapProcessor::ReadHeightPixel(uint8_t* row, UINT x, DXGI_FORMAT format, const Settings& settings)
{

    switch (format)
    {
    case DXGI_FORMAT_R32G32B32_FLOAT:
    {
        float* src = (float*)row;
        return src[x * 3 + 0];
    }

    case DXGI_FORMAT_R8G8B8A8_UNORM:
    {
        return Math::Vector3(static_cast<float>(row[x * 4 + 0]) / 255.0f, static_cast<float>(row[x * 4 + 1]) / 255.0f, static_cast<float>(row[x * 4 + 2]) / 255.0f).Length();
    }

    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    {
        uint8_t r = row[x * 4 + 0];
        float srgb = r / 255.0f;

        // Convert sRGB → linear
        return powf(srgb, 2.2f);
    }

    default:
        return 0.0f;
    }
}

void HeightmapProcessor::GaussianBlur(const std::vector<float>& input, std::vector<float>& output, int width, int height, int radius, float sigma)
{
    std::vector<float> kernel = CreateKernel(radius, sigma);

    std::vector<float> temp(width * height);
    output.resize(width * height);

    // --- Horizontal pass ---
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float sum = 0.0f;

            for (int k = -radius; k <= radius; ++k)
            {
                int sx = std::clamp(x + k, 0, width - 1);
                sum += input[y * width + sx] * kernel[k + radius];
            }

            temp[y * width + x] = sum;
        }
    }

    // --- Vertical pass ---
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float sum = 0.0f;

            for (int k = -radius; k <= radius; ++k)
            {
                int sy = std::clamp(y + k, 0, height - 1);
                sum += temp[sy * width + x] * kernel[k + radius];
            }

            output[y * width + x] = sum;
        }
    }
}

std::vector<float> HeightmapProcessor::CreateKernel(int radius, float sigma)
{
    std::vector<float> kernel(radius * 2 + 1);

    float sum = 0.0f;

    for (int i = -radius; i <= radius; ++i)
    {
        float value = expf(-(i * i) / (2.0f * sigma * sigma));
        kernel[i + radius] = value;
        sum += value;
    }

    // Normalize
    for (float& v : kernel)
        v /= sum;

    return kernel;
}
