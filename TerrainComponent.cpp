#include "TerrainComponent.h"

#include "iRender.h"
#include "Mesh.h"
#include "EditorUtils.h"
#include <DirectXCollision.h>
#include "TextureManager.h"
#include "StringUtils.h"
TerrainComponent::TerrainComponent(GameObject* owner)
	: Component(owner)
    , m_brushSize(30.0f)
{
    m_componentName = "TerrainComponent";
    m_componentType = ComponentType_TerrainComponent;

    m_mesh = std::make_shared<Mesh>();
    m_mesh->SetName("Terrain");


    CreatePlaneMesh();
}

void TerrainComponent::Render()
{
    Render::DrawInstance(m_mesh, m_owner->GetMatrix());
}

void TerrainComponent::Update(float deltaTime)
{
     float direction = ImGui::IsKeyDown(ImGuiKey_LeftShift) ?  1.0f : 0.0f;
     direction = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ?  -1.0f : direction;

    if ( !ImGuizmo::IsOver() && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGui::IsAnyItemHovered())
    {
        Math::Vector3 hitPosition;
        bool hitMesh = false;

        auto viewProj = Render::GetViewMatrix() * Render::GetProjectionMatrix();

        auto mousePos = ImGui::GetMousePos();
        auto windowSize = Render::GetWindowSize();

        if (mousePos.x > 0 && mousePos.x < windowSize.x && mousePos.y > 0 && mousePos.y < windowSize.y)
        {
            float ndcX = (2.0f * abs(mousePos.x)) / windowSize.x - 1.0f;
            float ndcY = 1.0f - (2.0f * abs(mousePos.y)) / windowSize.y;

            DirectX::XMMATRIX inverseviewproj = DirectX::XMMatrixInverse(nullptr, *static_cast<DirectX::XMMATRIX*>((void*)&viewProj));
            DirectX::XMVECTOR origin = DirectX::XMVectorSet(ndcX, ndcY, 0.0f, 1.0);
            DirectX::XMVECTOR farPoint = DirectX::XMVectorSet(ndcX, ndcY, 1.0f, 1.0);

            DirectX::XMVECTOR rayorigin = DirectX::XMVector3TransformCoord(origin, inverseviewproj);
            DirectX::XMVECTOR rayend = DirectX::XMVector3TransformCoord(farPoint, inverseviewproj);
            DirectX::XMVECTOR raydirection = DirectX::XMVectorSubtract(rayend, rayorigin);
            raydirection = DirectX::XMVector3Normalize(raydirection);

            float minDistance = std::numeric_limits<float>::max();

            const auto& mesh = m_mesh;
            DirectX::XMMATRIX inverseObjectMatrix = DirectX::XMMatrixInverse(nullptr, *static_cast<DirectX::XMMATRIX*>((void*)&m_owner->GetMatrix()));
            DirectX::XMVECTOR localRayOrigin = DirectX::XMVector3TransformCoord(rayorigin, inverseObjectMatrix);
            DirectX::XMVECTOR localRayDirection = DirectX::XMVector3TransformNormal(raydirection, inverseObjectMatrix);
            localRayDirection = DirectX::XMVector3Normalize(localRayDirection);
            for (unsigned int i = 0; i < mesh->indices32.size() - 3; i += 3)
            {
                int index1 = mesh->indices32[i];
                int index2 = mesh->indices32[(size_t)i + 1];
                int index3 = mesh->indices32[(size_t)i + 2];
                DirectX::XMVECTOR p1 = DirectX::XMVectorSet(mesh->vertexes[index1].position.x, mesh->vertexes[index1].position.y, mesh->vertexes[index1].position.z, 1.0f);
                DirectX::XMVECTOR p2 = DirectX::XMVectorSet(mesh->vertexes[index2].position.x, mesh->vertexes[index2].position.y, mesh->vertexes[index2].position.z, 1.0f);
                DirectX::XMVECTOR p3 = DirectX::XMVectorSet(mesh->vertexes[index3].position.x, mesh->vertexes[index3].position.y, mesh->vertexes[index3].position.z, 1.0f);

                float fakeDistance = 0.0f;
                if (DirectX::TriangleTests::Intersects(localRayOrigin, localRayDirection, p1, p2, p3, fakeDistance))
                { // i need to make this much better
                    auto dxVec = DirectX::XMVectorAdd(localRayOrigin, DirectX::XMVectorMultiply(localRayDirection, DirectX::XMVectorSet(fakeDistance, fakeDistance, fakeDistance, fakeDistance)));
                    hitPosition =  *static_cast<Math::Vector3*>((void*)&dxVec);
                    hitMesh = true;
                }
            }
        }
        if (direction != 0.0f && ImGui::IsKeyDown(ImGuiKey_MouseLeft) && hitMesh)
        {
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
            Render::CreateVertexAndIndexBuffer(m_mesh.get());
        }
    }
}
#ifdef EDITOR
void TerrainComponent::ComponentEditor()
{
    ImGui::DragFloat("BrushSize",&m_brushSize,0.7f,0.0f,100.0f);

    ImGui::Separator();

    struct TextureSelector
    {
        TextureSelector(std::string name, Editor::PropertyWindowType propertyType, int textureId)
            : m_name(name)
            , m_propertyType(propertyType)
            , m_textureId(textureId)
        {}
        std::string m_name;
        Editor::PropertyWindowType m_propertyType;
        int m_textureId;
    };
    std::vector<TextureSelector> textures{
        TextureSelector("Albedo", Editor::PropertyWindowType_Texture,ALBEDO),
        TextureSelector("Normal", Editor::PropertyWindowType_Normal,NORMAL),
        TextureSelector("Roughness", Editor::PropertyWindowType_Roughness,ROUGHNESS),
        TextureSelector("Metallic", Editor::PropertyWindowType_Metallic,METALLIC),
        TextureSelector("Ao", Editor::PropertyWindowType_Ao,AO),
    };

    for (const auto& texture : textures)
    {
        if (m_mesh->GetMaterial()->GetTexture(texture.m_textureId) && m_mesh->GetMaterial()->GetTexture(texture.m_textureId)->GetResourceView().Get())
        {
            ImGui::Text(std::string(texture.m_name + ": " + StringUtils::StripPathFromFilename(TextureManager::GetInstance()->GetTextureName(m_mesh->GetMaterial()->GetTexture(texture.m_textureId)))).c_str());
            auto resource = m_mesh->GetMaterial()->GetTexture(texture.m_textureId)->GetResourceView().Get();
            if (ImGui::ImageButton(resource, Editor::GetInstance()->GetDefaultTextureSize()) && m_mesh->GetMaterial()->GetName() != "default")
            {
                Editor::GetInstance()->OpenPropertyWindow(texture.m_propertyType);
            }
        }
        else
        {
            ImGui::Text(std::string(texture.m_name + ": Not selected").c_str());

            if (ImGui::Button(std::string("##" + texture.m_name).c_str(), Editor::GetInstance()->GetDefaultTextureSize()))
            {
                Editor::GetInstance()->OpenPropertyWindow(texture.m_propertyType);
            }
        }
        ImGui::Separator();
    }

    ImGui::DragFloat("Roughness", &m_mesh->GetMaterial()->GetMaterialSettings().Roughness, 1.0f, 0.0f, 1.0f);
    ImGui::DragFloat("Metallic", &m_mesh->GetMaterial()->GetMaterialSettings().Metallic, 1.0f, 0.0f, 1.0f);

}
#endif

void TerrainComponent::CreatePlaneMesh()
{
    const int nx = 100; // Number of segments along the x-axis
    const int ny = 100; // Number of segments along the y-axis
    const float width = 100.0f; // Width of the plane
    const float height = 100.0f; // Height of the plane
    const float UVscale = 10.0f;

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

            m_mesh->vertexes.back().position = DirectX::XMFLOAT3(x, y, z);
            m_mesh->vertexes.back().normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
            m_mesh->vertexes.back().tangent = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
            m_mesh->vertexes.back().uv = DirectX::XMFLOAT2(static_cast<float>(i) / nx * UVscale, static_cast<float>(j) / ny * UVscale);
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
    Render::CreateVertexAndIndexBuffer(m_mesh.get());
}
