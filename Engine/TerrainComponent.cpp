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

TerrainComponent::TerrainComponent(GameObject* owner)
	: Component(owner)
{

    m_mesh = std::make_shared<Mesh>();
    m_mesh->m_filename = "Terrain";
    m_material = std::make_shared<TerrainMaterial>();

    CreatePlaneMesh();



}

void TerrainComponent::Render()
{
    DirectX::BoundingBox boundingBox;



    auto camera = static_cast<PerspectiveCamera*>(Render::GetCamera());

    // 1. Build the projection matrix
    DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(
        camera->m_fov,
        camera->m_aspectRatio,
        camera->m_nearClip,
        camera->m_farClip
    );

    // 2. Create the view frustum
    DirectX::BoundingFrustum frustum;
    DirectX::BoundingFrustum::CreateFromMatrix(frustum, projMatrix);

    // 3. Transform it into world space
    DirectX::XMMATRIX worldMatrix = Spectral::DxMathUtils::ToDx(camera->GetWorldMatrix());

    DirectX::BoundingFrustum worldFrustum;
    frustum.Transform(worldFrustum, worldMatrix);



    Render::DrawInstance(m_mesh, m_material, m_owner->GetWorldMatrix());
    for (size_t i = 0; i < m_grassPatches.size(); i++)
    {
        DirectX::BoundingBox::CreateFromPoints(boundingBox, Spectral::DxMathUtils::ToDx(m_grassPatches[i].BoundingMin), Spectral::DxMathUtils::ToDx(m_grassPatches[i].BoundingMax));
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


    for (const auto& [textureName, materialId] : materials)
    {
        
        if (m_material->m_materials[materialId])
        {
            auto selectedMaterialName = m_material->m_materials[materialId]->GetFilename();
            ImGui::Text(std::string(textureName + ": " + selectedMaterialName).c_str());
            if (ImGui::Button(std::string("##" + textureName).c_str(), Editor::GetInstance()->GetDefaultTextureSize()))
            {
            }
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


    IOManager::SaveSpectralModel(GetMesh());

    return std::move(object);
}

void TerrainComponent::LoadComponent(const rapidjson::Value& object)
{
    m_mesh = ResourceManager::GetInstance()->GetResource<Mesh>("UserTerrain.model");

    
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


    auto device = Render::GetDevice();
    Math::Vector3 boundExtents = m_mesh->GetBoundingBoxMax() - m_mesh->GetBoundingBoxMin();

    int b = static_cast<int>(boundExtents.x / GrassPatch::PatchSize);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, GrassPatch::PatchSize);

    Math::Vector3 pos = m_owner->GetPosition();


    const int nx = 50; // Number of segments along the x-axis
    const int ny = 50; // Number of segments along the y-axis
    const float width = 500.0f; // Width of the plane
    const float height = 500.0f; // Height of the plane


    auto GetPosTerrainSurfaceHeight = [&](float x, float z)
    {
        float fx = (x / width) * nx;
        float fz = (z / height) * ny;

        int ix = static_cast<int>(floor(fx));
        int iz = static_cast<int>(floor(fz));

        float fracX = fx - ix;
        float fracZ = fz - iz;

        ix = std::clamp(ix, 0, nx - 1);
        iz = std::clamp(iz, 0, ny - 1);

        const int rowStride = nx + 1;

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


    for (int x = 0; x < b; x++)
    {
        for (int z = 0; z < b; z++)
        {
            GrassPatch patch{};
             
            std::vector<Math::Vector3> grassPositions(2000);

            patch.BoundingMin = Math::Vector3(static_cast<float>(x-3) * GrassPatch::PatchSize, 0.0f, static_cast<float>(z-3) * GrassPatch::PatchSize) + pos;
            patch.BoundingMax = patch.BoundingMin + Math::Vector3(GrassPatch::PatchSize*6, 100.0f, GrassPatch::PatchSize*6)+ pos;
            for (size_t i = 0; i < grassPositions.size(); i++)
            {
                grassPositions[i].x = dist(gen) + static_cast<float>(x) * GrassPatch::PatchSize;
                grassPositions[i].z = dist(gen) + static_cast<float>(z) * GrassPatch::PatchSize;


                grassPositions[i].y = GetPosTerrainSurfaceHeight(grassPositions[i].x, grassPositions[i].z);

                
                grassPositions[i] += pos;
                
            }
            patch.GrassPositionBufferData = Render::CreateVertexBuffer(device, grassPositions);
            patch.NumGrassPositions = static_cast<unsigned int>(grassPositions.size());
            m_grassPatches.push_back(patch);
        }
    }
}

std::shared_ptr<Mesh> TerrainComponent::GetMesh()
{
    return m_mesh;
}

void TerrainComponent::CreatePlaneMesh()
{
    const int nx = 50; // Number of segments along the x-axis
    const int ny = 50; // Number of segments along the y-axis
    const float width = 500.0f; // Width of the plane
    const float height = 500.0f; // Height of the plane
    const float UVscale = 50.0f;

    m_mesh->vertexes.reserve((size_t)nx * ny);
    m_mesh->indices32.reserve((size_t)nx * ny);

    float dx = width / nx;
    float dy = height / ny;
    for (int j = 0; j <= ny; ++j) {
        for (int i = 0; i <= nx; ++i) {
            float x = i * dx;
            float y = 0.0f;
            float z = j * dy;

            m_mesh->vertexes.emplace_back(
                Mesh::Vertex{
                    .position{ x, y, z },
                    .color{ 0x000000ff },
                    .uv{ static_cast<float>(i) / nx * UVscale, static_cast<float>(j) / ny * UVscale },
                    .normal{ 0.0f, 1.0f, 0.0f },
                    .tangent{ 1.0f, 0.0f, 0.0f },
                }
            );
        }
    }

    for (int j = 0; j < ny; ++j) {
        for (int i = 0; i < nx; ++i) {
            int row1 = j * (nx + 1);
            int row2 = (j + 1) * (nx + 1);

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
