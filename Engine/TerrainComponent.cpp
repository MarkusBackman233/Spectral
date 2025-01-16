#include "TerrainComponent.h"

#include "iRender.h"
#include "Mesh.h"
#include "EditorUtils.h"
#include <DirectXCollision.h>
#include "TextureManager.h"
#include "StringUtils.h"
#include "Editor.h"
#include "GameObject.h"
#ifdef EDITOR
#include "PropertyWindowFactory.h"
#endif // DEBUG
#include "EditorUtils.h"
#include "Intersection.h"
#include "IOManager.h"
#include "MaterialManager.h"
#include "Texture.h"
#include "Material.h"


TerrainComponent::TerrainComponent(GameObject* owner)
	: Component(owner)
    , m_brushSize(100.0f)
{

    m_mesh = std::make_shared<Mesh>();
    m_mesh->SetName("Terrain");


    CreatePlaneMesh();
}

void TerrainComponent::Render()
{
    Render::DrawInstance(m_mesh, m_owner->GetWorldMatrix());
}

void TerrainComponent::Update(float deltaTime)
{
#ifdef EDITOR
     float direction = ImGui::IsKeyDown(ImGuiKey_LeftShift) ?  1.0f : 0.0f;
     direction = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ?  -1.0f : direction;

    if (EditorUtils::IsCursorInViewport())
    {

        Math::Vector3 origin, rayDirection;
        if (!EditorUtils::CursorToWorldDirection(origin, rayDirection))
            return;
        float distance = 0.0f;

        if (direction != 0.0f && ImGui::IsKeyDown(ImGuiKey_MouseLeft))
        {
            if (!Spectral::Intersection::MeshTriangles(GetMesh().get(), m_owner->GetWorldMatrix(), origin, rayDirection, distance))
                return;

            auto hitPosition = (origin + rayDirection * distance).Transform(m_owner->GetWorldMatrix().GetInverse());


            for (auto& point : m_mesh->vertexes)
            {

                auto distanceVector = hitPosition - Math::Vector3(point.position.x, point.position.y, point.position.z);
                distanceVector.y = 0.0f;

                float distance = distanceVector.Length();

                if (distance < m_brushSize)
                {
                    point.position.y += (m_brushSize - distance) * 0.3f * direction * deltaTime;
                }
            }


            for (size_t i = 0; i < m_mesh->indices32.size(); i += 3) {
                Math::Vector3 v0 = m_mesh->vertexes[m_mesh->indices32[i]].position;
                Math::Vector3 v1 = m_mesh->vertexes[m_mesh->indices32[i + 1]].position;
                Math::Vector3 v2 = m_mesh->vertexes[m_mesh->indices32[i + 2]].position;

                Math::Vector3 edge1 = v1 - v0;
                Math::Vector3 edge2 = v2 - v0;
                
                Math::Vector3 normal = edge1.Cross(edge2).GetNormal();


                m_mesh->vertexes[m_mesh->indices32[i]].normal += normal;
                m_mesh->vertexes[m_mesh->indices32[i + 1]].normal += normal;
                m_mesh->vertexes[m_mesh->indices32[i + 2]].normal += normal;
            }

            for (size_t i = 0; i < m_mesh->vertexes.size(); ++i) 
            {
                m_mesh->vertexes[i].normal = -m_mesh->vertexes[i].normal;
            }

            m_mesh->CreateVertexAndIndexBuffer(Render::GetDevice());
            m_mesh->CalculateBoundingBox();
        }
    }
#endif // EDITOR
}
#ifdef EDITOR
void TerrainComponent::ComponentEditor()
{
    ImGui::DragFloat("BrushSize",&m_brushSize,0.7f,0.0f,100.0f);


    ImGui::Separator();


    ImGui::Text(std::string("Material: " + m_mesh->GetMaterial()->GetName()).c_str());
    if (ImGui::Button("Edit##Material"))
    {
        PropertyWindowFactory::SelectMaterial(m_mesh);
    }

    bool isDisabled = m_mesh->GetMaterial()->GetName() == "default";

    if (isDisabled)
    {
        ImGui::BeginDisabled();
    }

    enum TextureType
    {
        Albedo,
        Normal,
        Roughness,
        Metallic,
        AmbientOcclusion
    };

    static std::vector<std::pair<std::string, TextureType>> textures{
        {"Albedo", TextureType::Albedo},
        {"Normal", TextureType::Normal},
        {"Roughness", TextureType::Roughness},
        {"Metallic", TextureType::Metallic},
        {"Ao", TextureType::AmbientOcclusion},
    };

    for (const auto& [textureName, textureId] : textures)
    {
        if (m_mesh->GetMaterial()->GetTexture(textureId) && m_mesh->GetMaterial()->GetTexture(textureId)->GetResourceView().Get())
        {
            auto selectedTextureName = GetMesh()->GetMaterial()->GetTexture(textureId)->GetFilename();
            ImGui::Text(std::string(textureName + ": " + selectedTextureName).c_str());
            auto resource = m_mesh->GetMaterial()->GetTexture(textureId)->GetResourceView().Get();
            if (ImGui::ImageButton(textureName.c_str(), resource, Editor::GetInstance()->GetDefaultTextureSize()))
            {
                auto material = m_mesh->GetMaterial();
                PropertyWindowFactory::SelectTexture(material, textureId, selectedTextureName);
            }
        }
        else
        {
            ImGui::Text(std::string(textureName + ": Not selected").c_str());

            if (ImGui::Button(std::string("##" + textureName).c_str(), Editor::GetInstance()->GetDefaultTextureSize()))
            {
                auto material = m_mesh->GetMaterial();
                PropertyWindowFactory::SelectTexture(material, textureId);
            }
        }
        ImGui::Separator();
    }

    ImGui::DragFloat("Roughness", &m_mesh->GetMaterial()->GetMaterialSettings().Roughness, 0.05f, 0.0f, 1.0f);
    ImGui::DragFloat("Metallic", &m_mesh->GetMaterial()->GetMaterialSettings().Metallic, 0.05f, 0.0f, 1.0f);

    ImGui::ColorPicker4("Color", &m_mesh->GetMaterial()->GetMaterialSettings().Color.x, Editor::GetInstance()->ColorPickerMask);

    if (isDisabled)
    {
        ImGui::EndDisabled();
    }
}
void TerrainComponent::DisplayComponentIcon()
{
    ImGui::SameLine();
    ImGui::Image(TextureManager::GetInstance()->GetTexture("Terrain.bmp")->GetResourceView().Get(), ImVec2(15, 15));
}
#endif

void TerrainComponent::SaveComponent(rapidjson::Value& object, rapidjson::Document::AllocatorType& allocator)
{
    m_mesh->SetName("UserTerrain");

    object.AddMember("Terrain Name", rapidjson::Value(m_mesh->GetName().c_str(), allocator), allocator);
    IOManager::SaveSpectralModel(GetMesh());
    object.AddMember("Material", rapidjson::Value(m_mesh->GetMaterial()->GetName().c_str(), allocator), allocator);


}

void TerrainComponent::LoadComponent(const rapidjson::Value& object)
{
    IOManager::LoadSpectralModel("UserTerrain", m_mesh);
    m_mesh->SetMaterial(MaterialManager::GetInstance()->GetMaterial(object["Material"].GetString()));
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
            float y = 0.0f; // Assuming the plane is flat on the y=0 plane
            float z = j * dy;

            m_mesh->vertexes.push_back(Mesh::Vertex());

            m_mesh->vertexes.back().position = Math::Vector3(x, y, z);
            m_mesh->vertexes.back().normal = Math::Vector3(0.0f, 1.0f, 0.0f);
            m_mesh->vertexes.back().tangent = Math::Vector3(1.0f, 0.0f, 0.0f);
            m_mesh->vertexes.back().uv = Math::Vector2(static_cast<float>(i) / nx * UVscale, static_cast<float>(j) / ny * UVscale);
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
