#include "TerrainEditor.h"
#include <src/IMGUI/imgui.h>
#include <TerrainComponent.h>
#include <Vector3.h>
#include "EditorUtils.h"
#include <Intersection.h>
#include <GameObject.h>
#include <Mesh.h>
#include <TimeManager.h>
#include <iRender.h>
#include <RenderManager.h>
#include <TerrainMaterial.h>

TerrainEditor::TerrainEditor(TerrainComponent* terrainComponent)
	: m_terrainComponent(terrainComponent)
	, m_terrainDirty(false)
{
}

bool TerrainEditor::Update()
{
    TerrainMaterial::g_brushSize = m_settings.BrushSize;

    bool stayOpen = true;

	RaycastPaint();
	if (ImGui::Begin("Terrain Editor", &stayOpen))
	{
        ImGui::SeparatorText("Brush");
		ImGui::SliderFloat("Size##Brush", &m_settings.BrushSize, 1.0f, 100.0f);
        ImGui::SliderFloat("Strength##Brush", &m_settings.BrushStrength, 1.0f, 100.0f);

        constexpr const char* BrushModeNames[] = { "Height", "Color" };

        int currentBrushMode = static_cast<int>(m_settings.BrushMode);
        if (ImGui::Combo("Brush Mode", &currentBrushMode, BrushModeNames, BrushMode::Num))
        {
            m_settings.BrushMode = static_cast<BrushMode>(currentBrushMode);
        }

        switch (m_settings.BrushMode)
        {
        case BrushMode::Color:
        {
            ImGui::SliderInt("Selected Brush Material", &m_settings.SelectedBrushMaterial,0,3);

            break;
        }
        default:
            break;
        }


        if (ImGui::Button("Close Terrain Editor"))
        {
            stayOpen = false;
        }

		ImGui::End();
	}
    return stayOpen;
}

void TerrainEditor::RaycastPaint()
{
    float direction = ImGui::IsKeyDown(ImGuiKey_LeftShift) ? 1.0f : 0.0f;
    direction = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ? -1.0f : direction;
    std::shared_ptr<Mesh> mesh = m_terrainComponent->GetMesh();
    const Math::Matrix& worldMatrix = m_terrainComponent->GetOwner()->GetWorldMatrix();
    auto& vertexes = mesh->vertexes;
    auto& indices = mesh->indices32;

    Math::Vector3 origin, rayDirection;
    if (!EditorUtils::CursorToWorldDirection(origin, rayDirection))
        return;

    float distance = 0.0f;


    if (!Spectral::Intersection::MeshTriangles(mesh.get(), worldMatrix, origin, rayDirection, distance))
        return;
    
    Math::Vector3 hitPosition = (origin + rayDirection * distance);
    Math::Vector3 localHitPosition = hitPosition.Transform(worldMatrix.GetInverse());
    
    TerrainMaterial::g_mouseRayHit = hitPosition;

    if (ImGui::IsKeyDown(ImGuiKey_MouseLeft))
    {
        m_terrainDirty = true;
        for (auto& point : vertexes)
        {
            auto distanceVector = localHitPosition - Math::Vector3(point.position.x, point.position.y, point.position.z);
            distanceVector.y = 0.0f;

            float distance = distanceVector.Length();

            if (distance < m_settings.BrushSize)
            {
                float t = 1.0f - (distance / m_settings.BrushSize);
                t = std::clamp(t, 0.0f, 1.0f);

                float influence = 1.0f - (1.0f - t) * (1.0f - t);
                if (m_settings.BrushMode == BrushMode::Color)
                {

                    //uint32_t color = point.color;
                    //uint8_t r = (color >> 0)  & 0xFF;
                    //uint8_t g = (color >> 8)  & 0xFF;
                    //uint8_t b = (color >> 16) & 0xFF;
                    //uint8_t a = (color >> 24) & 0xFF;


                    //uint8_t r = static_cast<uint8_t>(m_settings.BrushStrength), g = 0, b = 0, a = 255;
                    //point.color = (r) | (g << 8) | (b << 16) | (a << 24);

                    uint32_t color = point.color;

                    int a = (color >> 24) & 0xFF;
                    int b = (color >> 16) & 0xFF;
                    int g = (color >> 8) & 0xFF;
                    int r = (color) & 0xFF;

                    switch (m_settings.SelectedBrushMaterial)
                    {
                    case 0:
                        r += 1 * static_cast<int>(m_settings.BrushStrength * direction);
                        r = std::clamp(r, 0, 255);
                        if (direction > 0.0f)
                        {
                            g -= 1 * static_cast<int>(m_settings.BrushStrength);
                            g = std::clamp(g, 0, 255);

                            b -= 1 * static_cast<int>(m_settings.BrushStrength);
                            b = std::clamp(b, 0, 255);

                            a -= 1 * static_cast<int>(m_settings.BrushStrength);
                            a = std::clamp(a, 0, 255);
                        }

                        break;
                    case 1:
                        g += 1 * static_cast<int>(m_settings.BrushStrength * direction);
                        g = std::clamp(g, 0, 255);

                        if (direction > 0.0f)
                        {
                            r -= 1 * static_cast<int>(m_settings.BrushStrength);
                            r = std::clamp(r, 0, 255);

                            b -= 1 * static_cast<int>(m_settings.BrushStrength);
                            b = std::clamp(b, 0, 255);

                            a -= 1 * static_cast<int>(m_settings.BrushStrength);
                            a = std::clamp(a, 0, 255);
                        }
                        break;
                    case 2:
                        b += 1 * static_cast<int>(m_settings.BrushStrength * direction);
                        b = std::clamp(b, 0, 255);

                        if (direction > 0.0f)
                        {
                            g -= 1 * static_cast<int>(m_settings.BrushStrength);
                            g = std::clamp(g, 0, 255);

                            r -= 1 * static_cast<int>(m_settings.BrushStrength);
                            r = std::clamp(r, 0, 255);

                            a -= 1 * static_cast<int>(m_settings.BrushStrength);
                            a = std::clamp(a, 0, 255);
                        }
                        break;
                    case 3:
                        a += 1 * static_cast<int>(m_settings.BrushStrength * direction);
                        a = std::clamp(a, 0, 255);

                        if (direction > 0.0f)
                        {
                            g -= 1 * static_cast<int>(m_settings.BrushStrength);
                            g = std::clamp(g, 0, 255);

                            b -= 1 * static_cast<int>(m_settings.BrushStrength);
                            b = std::clamp(b, 0, 255);

                            r -= 1 * static_cast<int>(m_settings.BrushStrength);
                            r = std::clamp(r, 0, 255);
                        }
                        break;
                    }


                    point.color = (static_cast<uint8_t>(a) << 24) | (static_cast<uint8_t>(b) << 16) | (static_cast<uint8_t>(g) << 8) | static_cast<uint8_t>(r);
                }

                if (m_settings.BrushMode == BrushMode::Height)
                {
                    point.position.y += influence * direction * TimeManager::GetDeltaTime() * m_settings.BrushStrength;
                }
            }
        }
        mesh->CreateVertexAndIndexBuffer(Render::GetDevice());
        mesh->CalculateBoundingBox();
    }
    else if (m_terrainDirty)
    {

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



        mesh->CreateVertexAndIndexBuffer(Render::GetDevice());
        m_terrainDirty = false;
    }

}