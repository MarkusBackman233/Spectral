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
#include <MathFunctions.h>
#include <set>
TerrainEditor::TerrainEditor(TerrainComponent* terrainComponent)
	: m_terrainComponent(terrainComponent)
	, m_terrainDirty(false)
    , m_gen(m_randomDevice())
    , m_randomAngleDist(0.0f, 2.0f * Math::PI)
{
    m_grassPatchStride = static_cast<int>(m_terrainComponent->GetTerrainSize() / GrassPatch::PatchSize);
}

bool TerrainEditor::Update()
{
    TerrainMaterial::g_brushSize = m_settings.BrushSize;

    bool stayOpen = true;

	RaycastPaint();
	if (ImGui::Begin("Terrain Editor", &stayOpen))
	{

        constexpr const char* BrushModeNames[] = { "Height", "Color", "Grass"};

        int currentBrushMode = static_cast<int>(m_settings.BrushMode);
        if (ImGui::Combo("Brush Mode", &currentBrushMode, BrushModeNames, BrushMode::Num))
        {
            m_settings.BrushMode = static_cast<BrushMode>(currentBrushMode);
        }

        switch (m_settings.BrushMode)
        {
        case BrushMode::Color:
        {
            ImGui::SeparatorText("Brush");
            ImGui::SliderFloat("Size##Brush", &m_settings.BrushSize, 1.0f, 100.0f);
            ImGui::SliderFloat("Strength##Brush", &m_settings.BrushStrength, 1.0f, 100.0f);
            ImGui::SliderInt("Selected Brush Material", &m_settings.SelectedBrushMaterial,0,3);

            break;
        }
        case BrushMode::Height:
        {
            ImGui::SeparatorText("Brush");
            ImGui::SliderFloat("Size##Brush", &m_settings.BrushSize, 1.0f, 100.0f);
            ImGui::SliderFloat("Strength##Brush", &m_settings.BrushStrength, 1.0f, 100.0f);

            break;
        }
        case BrushMode::Grass:
        {
            ImGui::SeparatorText("Brush");
            ImGui::SliderFloat("Size##Brush", &m_settings.BrushSize, 1.0f, 100.0f);
            ImGui::SliderFloat("Grass radius##Brush", &m_settings.BrushGrassRadius, 0.5f, 3.0f);

            break;
        }
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
    float direction = 0.0f;
    if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) 
        direction = 1.0f;
    else if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) 
        direction = -1.0f;

    Mesh* terrainMesh = m_terrainComponent->GetMesh().get();
    std::optional<Math::Vector3> optHit = RaycastTerrain(terrainMesh);

    if (!optHit.has_value())
    {
        return;
    }
    Math::Vector3 localHit = optHit.value();

    if (ImGui::IsKeyDown(ImGuiKey_MouseLeft))
    {
        switch (m_settings.BrushMode)
        {
        case BrushMode::Height:
            EditHeight(terrainMesh, localHit, direction);
            break;
        case BrushMode::Color:
            EditSplat(terrainMesh, localHit, direction);
            break;
        case BrushMode::Grass:
            EditGrass(terrainMesh, localHit, direction);
            break;
        }
    }
    else if (m_terrainDirty)
    {
        m_terrainComponent->BuildTerrain();
        //RebuildTerrain(terrainMesh);
    }
}

void TerrainEditor::EditHeight(Mesh* terrainMesh, const Math::Vector3& localHit, float direction)
{

    std::set<GrassPatch*> changedPatches;

    auto& vertexes = terrainMesh->vertexes;
    auto& indices = terrainMesh->indices32;
    for (auto& point : vertexes)
    {
        auto distanceVector = localHit - Math::Vector3(point.position.x, point.position.y, point.position.z);
        distanceVector.y = 0.0f;

        float distance = distanceVector.Length();

        if (distance < m_settings.BrushSize)
        {
            float t = 1.0f - (distance / m_settings.BrushSize);
            t = std::clamp(t, 0.0f, 1.0f);

            float influence = 1.0f - (1.0f - t) * (1.0f - t);
            m_terrainDirty = true;

            point.position.y += influence * direction * TimeManager::GetDeltaTime() * m_settings.BrushStrength;


            int x = static_cast<int>(std::floor(point.position.x / GrassPatch::PatchSize));
            int z = static_cast<int>(std::floor(point.position.z / GrassPatch::PatchSize));

            for (int gx = -1; gx < 1; gx++)
            {
                for (int gz = -1; gz < 1; gz++)
                {
                    GrassPatch* patch = GetGrassPatchFromCoords(x + gx, z + gz);

                    if (patch)
                    {
                        changedPatches.insert(patch);
                    }
                }
            }
        }
    }
    for (GrassPatch* patch : changedPatches)
    {
        m_terrainComponent->GenerateGrass(*patch);
    }

    if (m_terrainDirty)
    {
        terrainMesh->CreateVertexAndIndexBuffer(Render::GetDevice());
        terrainMesh->CalculateBoundingBox();
    }
}

void TerrainEditor::EditGrass(Mesh* terrainMesh, const Math::Vector3& localHit, float direction)
{
    if (direction > 0.0f)
    {
        AddGrass(localHit);
        return;
    }

    if (direction < 0.0f)
    {
        RemoveGrass(localHit);
        return;
    }
}

GrassPatch* TerrainEditor::GetGrassPatchFromWorldSpace(const Math::Vector3& pos)
{
    int x = static_cast<int>(std::floor(pos.x / GrassPatch::PatchSize));
    int z = static_cast<int>(std::floor(pos.z / GrassPatch::PatchSize));
    return GetGrassPatchFromCoords(x, z);
};

void TerrainEditor::AddGrass(const Math::Vector3& localHit)
{

    std::set<GrassPatch*> changedPatches;
    std::vector<Math::Vector3> nearbyTufts;


    auto nearbyPatches = GetGrassPatchesInRadius(localHit, m_settings.BrushSize);
    for(GrassPatch* patch : nearbyPatches)
    {
        bool changed = false;
        auto& tufts = patch->GrassTufts;
        tufts.erase(std::remove_if(tufts.begin(), tufts.end(),
            [&](const Math::Vector3& tuft)
        {
            for (const auto& tuft2 : tufts)
            {
                if (tuft.HorizontalLength(tuft2) < m_settings.BrushGrassRadius && 
                    localHit.HorizontalLength(tuft2) < m_settings.BrushSize && 
                    tuft2 != tuft)
                {
                    changed = true;
                    return true;
                }
            }
            return false;
        }),
            tufts.end());


        if (changed)
        {
            changedPatches.insert(patch);
        }

        for (const auto& tuft : tufts)
        {
            if (tuft.HorizontalLength(localHit) < m_settings.BrushSize)
            {
                nearbyTufts.push_back(tuft);
            }
        }
    };


    if (nearbyTufts.empty())
    {
        auto patch = GetGrassPatchFromWorldSpace(localHit);
        if (patch)
        {
            patch->GrassTufts.push_back(localHit);
            nearbyTufts.push_back(localHit);
            changedPatches.insert(patch);
        }
    }

    std::deque<uint32_t> activeList;
    uint32_t nbNearbyTufts = static_cast<uint32_t>(nearbyTufts.size());
    for (uint32_t i = 0; i < nbNearbyTufts; i++)
    {
        activeList.push_back(i);
    }

    std::uniform_real_distribution<float> rDist(m_settings.BrushGrassRadius, m_settings.BrushGrassRadius * 2.0f);

    while (!activeList.empty())
    {
        const auto& activeTuft = nearbyTufts[activeList.front()];

        for (uint8_t k = 0; k < 30; k++)
        {
            float angle = m_randomAngleDist(m_gen);
            Math::Vector3 dir(std::cos(angle), 0.0f, std::sin(angle));
            Math::Vector3 p = dir * rDist(m_gen) + activeTuft;

            bool good = true;

            for (const auto& tuft : nearbyTufts)
            {
                if (tuft.HorizontalLength(p) < m_settings.BrushGrassRadius || 
                    p.HorizontalLength(localHit) > m_settings.BrushSize)
                {
                    good = false;
                    break;
                }
            }

            if (good)
            {
                GrassPatch* patch = GetGrassPatchFromWorldSpace(p);
                if (patch)
                {
                    activeList.push_back(static_cast<uint32_t>(nearbyTufts.size()));
                    nearbyTufts.push_back(p);
                    patch->GrassTufts.push_back(p);
                    changedPatches.insert(patch);
                    break;
                }
            }

        }
        activeList.pop_front();
    }

    for (GrassPatch* patch : changedPatches)
    {
        m_terrainComponent->GenerateGrass(*patch);
    }
}

void TerrainEditor::RemoveGrass(const Math::Vector3& localHit)
{
    std::set<GrassPatch*> changedPatches;

    auto patches = GetGrassPatchesInRadius(localHit, m_settings.BrushSize);

    for (GrassPatch* patch : patches)
    {
        bool changed = false;
        auto& tufts = patch->GrassTufts;
        tufts.erase(std::remove_if(tufts.begin(), tufts.end(),
            [&](const Math::Vector3& tuft)
        {
            if (localHit.HorizontalLength(tuft) < m_settings.BrushSize)
            {
                changed = true;
                return true;
            }
            return false;
        }),
            tufts.end());


        if (changed)
        {
            changedPatches.insert(patch);
        }
    }


    for (GrassPatch* patch : changedPatches)
    {
        m_terrainComponent->GenerateGrass(*patch);
    }
}

GrassPatch* TerrainEditor::GetGrassPatchFromCoords(int x, int z)
{
    if (x < 0 || x >= m_grassPatchStride)
    {
        return nullptr;
    }
    if (z < 0 || z >= m_grassPatchStride)
    {
        return nullptr;
    }

    return &m_terrainComponent->m_grassPatches[static_cast<size_t>(x) * m_grassPatchStride + z];
}

std::vector<GrassPatch*> TerrainEditor::GetGrassPatchesInRadius(const Math::Vector3& center, float radius)
{
    std::vector<GrassPatch*> patches;

    int centerx = static_cast<int>(std::floor(center.x / GrassPatch::PatchSize));
    int centerz = static_cast<int>(std::floor(center.z / GrassPatch::PatchSize));

    int brushSizeInPatches = static_cast<int>(std::ceil(radius / GrassPatch::PatchSize)) * 2;

    for (int x = -brushSizeInPatches; x < brushSizeInPatches; x++)
    {
        for (int z = -brushSizeInPatches; z < brushSizeInPatches; z++)
        {
            auto patch = GetGrassPatchFromCoords(x + centerx, z + centerz);
            if (patch)
            {
                patches.push_back(patch);
            }
        }
    }
    return patches;
}

std::optional<Math::Vector3> TerrainEditor::RaycastTerrain(Mesh* terrainMesh)
{
    const Math::Matrix& worldMatrix = m_terrainComponent->GetOwner()->GetWorldMatrix();

    Math::Vector3 origin, rayDirection;
    if (!EditorUtils::CursorToWorldDirection(origin, rayDirection))
        return std::nullopt;

    float distance = 0.0f;
    if (!Spectral::Intersection::MeshTriangles(terrainMesh, worldMatrix, origin, rayDirection, distance))
        return std::nullopt;

    Math::Vector3 hit = (origin + rayDirection * distance);
    Math::Vector3 localHit = hit.Transform(worldMatrix.GetInverse());

    TerrainMaterial::g_mouseRayHit = hit;


    return localHit;
}

void TerrainEditor::EditSplat(Mesh* terrainMesh, const Math::Vector3& localHit, float direction)
{
    auto& vertexes = terrainMesh->vertexes;
    auto& indices = terrainMesh->indices32;
    for (auto& point : vertexes)
    {
        auto distanceVector = localHit - Math::Vector3(point.position.x, point.position.y, point.position.z);
        distanceVector.y = 0.0f;

        float distance = distanceVector.Length();

        if (distance < m_settings.BrushSize)
        {
            m_terrainDirty = true;

            float t = 1.0f - (distance / m_settings.BrushSize);
            t = std::clamp(t, 0.0f, 1.0f);

            float influence = 1.0f - (1.0f - t) * (1.0f - t);

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
    }
    if (m_terrainDirty)
    {
        terrainMesh->CreateVertexAndIndexBuffer(Render::GetDevice());
        terrainMesh->CalculateBoundingBox();
    }
}

