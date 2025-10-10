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

TerrainComponent::TerrainComponent(GameObject* owner)
	: Component(owner)
    , m_vertexRowCount(50)
    , m_uvScale(50)
    , m_terrainSize(500.0f)
{

    m_mesh = std::make_shared<Mesh>();
    m_mesh->m_filename = "Terrain";
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



    Render::DrawInstance(m_mesh, m_material, m_owner->GetWorldMatrix());
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
    if (m_mesh->vertexes.empty())
    {
        static int vertexRowCount = 50ull;
        static float terrainSize = 500.0f;

        ImGui::InputFloat("Size", &terrainSize);
        ImGui::InputInt("Resolution", &vertexRowCount);
        if (ImGui::Button("Create Terrain") && vertexRowCount > 2 && terrainSize > 10)
        {
            m_vertexRowCount = static_cast<size_t>(vertexRowCount);
            m_terrainSize = terrainSize;
            vertexRowCount = 50ull;
            terrainSize = 500.0f;
            CreatePlaneMesh();
            Math::Vector3 pos = m_owner->GetPosition();
            int b = static_cast<int>(m_terrainSize / GrassPatch::PatchSize);
            for (int x = 0; x < b; x++)
            {
                for (int z = 0; z < b; z++)
                {
                    GrassPatch patch{};
                    patch.BoundingMin = Math::Vector3(static_cast<float>(x) * GrassPatch::PatchSize, 0.0f, static_cast<float>(z) * GrassPatch::PatchSize) + pos;
                    patch.BoundingMax = patch.BoundingMin + Math::Vector3(GrassPatch::PatchSize, 100.0f, GrassPatch::PatchSize) + pos;
                    m_grassPatches.push_back(patch);
                }
            }
            m_material->m_materials[0] = ResourceManager::GetInstance()->GetResource<DefaultMaterial>("Default.material");

        }

        return;
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

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSETS_BROWSER_ITEMS"))
            {
                size_t size = payload->DataSize / sizeof(FileItem);
                FileItem* payload_items = (FileItem*)payload->Data;
                for (size_t i = 0; i < size; i++)
                {
                    FileItem& item = payload_items[i];
                    if (item.m_type != ResourceType::Material)
                    {
                        continue;
                    }
                    auto droppedTexture = ResourceManager::GetInstance()->GetResource<DefaultMaterial>(item.m_filename);

                    if (droppedTexture)
                    {
                        m_material->m_materials[materialId] = droppedTexture;
                        break;
                    }
                }
            }
            ImGui::EndDragDropTarget();
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

    m_mesh->m_filename = "UserTerrain.model";
    object.emplace("Terrain Name", m_mesh->GetFilename());

    object.emplace("Material0", m_material->m_materials[0] ? m_material->m_materials[0]->GetFilename() : "null");
    object.emplace("Material1", m_material->m_materials[1] ? m_material->m_materials[1]->GetFilename() : "null");
    object.emplace("Material2", m_material->m_materials[2] ? m_material->m_materials[2]->GetFilename() : "null");
    object.emplace("Material3", m_material->m_materials[3] ? m_material->m_materials[3]->GetFilename() : "null");


    //IOManager::SaveSpectralModel(GetMesh());

    auto path = IOManager::ProjectDirectory / IOManager::GetResourceData<ResourceType::Model>().Folder / "UserTerrain.terrain";
    WriteObject out(path);

    uint32_t version = 1;

    out.Write(version);
    out.Write(m_vertexRowCount);
    out.Write(m_terrainSize);
    out.Write(m_uvScale);
    for (int i = 0; i < m_mesh->vertexes.size(); ++i)
    {
        out.Write(
            TerrainPointV1{
                .Height =  m_mesh->vertexes[i].position.y, 
                .Color =  m_mesh->vertexes[i].color
            }
        );
    }


    out.Write(m_grassPatches.size());
    for (const GrassPatch& patch : m_grassPatches)
    {
        out.Write(patch.GrassTufts);

    }
    return std::move(object);
}

void TerrainComponent::LoadComponent(const rapidjson::Value& object)
{
    ReadObject in(IOManager::ProjectDirectory / IOManager::GetResourceData<ResourceType::Model>().Folder / "UserTerrain.terrain");
    if (in.GetFile().is_open())
    {
        uint32_t version;
        in.Read(version);
        in.Read(m_vertexRowCount);
        in.Read(m_terrainSize);
        in.Read(m_uvScale);

        float dx = m_terrainSize / m_vertexRowCount;
        float dy = m_terrainSize / m_vertexRowCount;
        for (size_t j = 0; j <= m_vertexRowCount; j++)
        {
            for (size_t i = 0; i <= m_vertexRowCount; i++)
            {
                TerrainPointV1 point;
                in.Read(point);

                float x = i * dx;
                float y = point.Height;
                float z = j * dy;

                m_mesh->vertexes.emplace_back(
                    Mesh::Vertex{
                        .position{ x, y, z },
                        .color{ point.Color },
                        .uv{ static_cast<float>(i) / m_vertexRowCount * m_uvScale, static_cast<float>(j) / m_vertexRowCount * m_uvScale },
                        .normal{ 0.0f, 1.0f, 0.0f },
                        .tangent{ 1.0f, 0.0f, 0.0f },
                    }
                    );
            }
        }
        for (int j = 0; j < m_vertexRowCount; ++j) {
            for (int i = 0; i < m_vertexRowCount; ++i) {
                int row1 = j * (m_vertexRowCount + 1);
                int row2 = (j + 1) * (m_vertexRowCount + 1);
                m_mesh->indices32.push_back(row1 + i);
                m_mesh->indices32.push_back(row1 + i + 1);
                m_mesh->indices32.push_back(row2 + i);

                m_mesh->indices32.push_back(row1 + i + 1);
                m_mesh->indices32.push_back(row2 + i + 1);
                m_mesh->indices32.push_back(row2 + i);
            }
        }


        size_t nbGrassPatches;
        in.Read(nbGrassPatches);
        int b = static_cast<int>(m_terrainSize / GrassPatch::PatchSize);
        
        Math::Vector3 pos = m_owner->GetPosition();
        
        for (int x = 0; x < b; x++)
        {
            for (int z = 0; z < b; z++)
            {
                GrassPatch patch{};
        
                patch.BoundingMin = Math::Vector3(static_cast<float>(x) * GrassPatch::PatchSize, 0.0f, static_cast<float>(z) * GrassPatch::PatchSize) + pos;
                patch.BoundingMax = patch.BoundingMin + Math::Vector3(GrassPatch::PatchSize, 100.0f, GrassPatch::PatchSize) + pos;
                in.Read(patch.GrassTufts);
                m_grassPatches.push_back(patch);
                if (!patch.GrassTufts.empty())
                {
                    GenerateGrass(m_grassPatches.back());
                }
            }
        }


        BuildTerrain();
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

std::shared_ptr<Mesh> TerrainComponent::GetMesh() const
{
    return m_mesh;
}

void TerrainComponent::GenerateGrass(GrassPatch& patch)
{
    if (patch.GrassPositionBufferData != nullptr)
        patch.GrassPositionBufferData.Reset();

    auto GetPosTerrainSurfaceHeight = [&](float x, float z)
    {
        float fx = x / m_terrainSize * m_vertexRowCount;
        float fz = z / m_terrainSize * m_vertexRowCount;

        int ix = static_cast<int>(floor(fx));
        int iz = static_cast<int>(floor(fz));

        float fracX = fx - ix;
        float fracZ = fz - iz;

        ix = std::clamp(ix, 0, static_cast<int>(m_vertexRowCount) - 1);
        iz = std::clamp(iz, 0, static_cast<int>(m_vertexRowCount) - 1);

        const int rowStride = m_vertexRowCount + 1;

        float h00 = m_mesh->vertexes[iz * rowStride + ix].position.y;
        float h10 = m_mesh->vertexes[iz * rowStride + (ix + 1)].position.y;
        float h01 = m_mesh->vertexes[(iz + 1) * rowStride + ix].position.y;
        float h11 = m_mesh->vertexes[(iz + 1) * rowStride + (ix + 1)].position.y;

        float height =
            (1.0f - fracX) * (1.0f - fracZ) * h00 +
            fracX * (1.0f - fracZ) * h10 +
            (1.0f - fracX) * fracZ * h01 +
            fracX * fracZ * h11;

        return height;
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

void TerrainComponent::CreatePlaneMesh()
{
    m_mesh->vertexes.reserve(m_vertexRowCount * m_vertexRowCount);
    m_mesh->indices32.reserve(m_vertexRowCount * m_vertexRowCount);

    float dx = m_terrainSize / m_vertexRowCount;
    float dy = m_terrainSize / m_vertexRowCount;
    for (int j = 0; j <= m_vertexRowCount; ++j) {
        for (int i = 0; i <= m_vertexRowCount; ++i) {
            float x = i * dx;
            float y = 0.0f;
            float z = j * dy;

            m_mesh->vertexes.emplace_back(
                Mesh::Vertex{
                    .position{ x, y, z },
                    .color{ 0x000000ff },
                    .uv{ static_cast<float>(i) / m_vertexRowCount * m_uvScale, static_cast<float>(j) / m_vertexRowCount * m_uvScale },
                    .normal{ 0.0f, 1.0f, 0.0f },
                    .tangent{ 1.0f, 0.0f, 0.0f },
                }
            );
        }
    }

    for (int j = 0; j < m_vertexRowCount; ++j) {
        for (int i = 0; i < m_vertexRowCount; ++i) {
            int row1 = j * (m_vertexRowCount + 1);
            int row2 = (j + 1) * (m_vertexRowCount + 1);

            // First triangle (i, j) -> (i+1, j) -> (i, j+1)
            m_mesh->indices32.push_back(row1 + i);
            m_mesh->indices32.push_back(row1 + i + 1);
            m_mesh->indices32.push_back(row2 + i);

            // Second triangle (i+1, j) -> (i+1, j+1) -> (i, j+1)
            m_mesh->indices32.push_back(row1 + i + 1);
            m_mesh->indices32.push_back(row2 + i + 1);
            m_mesh->indices32.push_back(row2 + i);
        }
    }


    m_mesh->CreateVertexAndIndexBuffer(Render::GetDevice());
    m_mesh->CalculateBoundingBox();
}

void TerrainComponent::BuildTerrain()
{
    auto& vertexes = m_mesh->vertexes;
    auto& indices = m_mesh->indices32;
    for (size_t i = 0; i < indices.size(); i += 3) {
        Math::Vector3 v0 = vertexes[indices[i]].position;
        Math::Vector3 v1 = vertexes[indices[i + 1]].position;
        Math::Vector3 v2 = vertexes[indices[i + 2]].position;

        Math::Vector3 edge1 = v1 - v0;
        Math::Vector3 edge2 = v2 - v0;

        Math::Vector3 normal = -edge1.Cross(edge2).GetNormal();


        vertexes[indices[i]].normal = normal;
        vertexes[indices[i + 1]].normal = normal;
        vertexes[indices[i + 2]].normal = normal;
    }
    if (m_mesh->m_pVertexBuffer)
    {
        m_mesh->m_pVertexBuffer.Reset();
        m_mesh->m_pVertexBuffer = nullptr;
    }
    if (m_mesh->m_pIndexBuffer)
    {
        m_mesh->m_pIndexBuffer.Reset();
        m_mesh->m_pIndexBuffer = nullptr;
    }
    m_mesh->CreateVertexAndIndexBuffer(Render::GetDevice());
    m_mesh->CalculateBoundingBox();
}

size_t TerrainComponent::GetVertexRowCount() const
{
    return m_vertexRowCount;
}

float TerrainComponent::GetTerrainSize() const
{
    return m_terrainSize;
}
