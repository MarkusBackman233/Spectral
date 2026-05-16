#include "VegetationComponent.h"
#include "src/IMGUI/imgui.h"
#include "iRender.h"
#include "GameObject.h"
#include "MathFunctions.h"
#include "Mesh.h"
#include "ResourceManager.h"
#include <Editor.h>
#include "DefaultMaterial.h"
#include "RenderManager.h"
VegetationComponent::VegetationComponent(GameObject* owner)
	: Component(owner)
{
    m_model = std::make_shared<Model>();
    m_model->m_filename = std::filesystem::path(GetOwner()->GetName()).replace_extension(IOManager::GetResourceData<ResourceType::Model>().SpectralExtension).string();

    m_model->m_root.m_mesh = std::make_shared<Mesh>("Foliage");
    m_model->GetMaterials().push_back(ResourceManager::GetInstance()->GetResource<DefaultMaterial>("Default.material"));
    m_model->GetMaterials().push_back(ResourceManager::GetInstance()->GetResource<DefaultMaterial>("Default.material"));
    Generate();

}

VegetationComponent::VegetationComponent(GameObject* owner, VegetationComponent* vegetationComponent)
	: Component(owner)
{
    m_model = vegetationComponent->m_model;
}

VegetationComponent::~VegetationComponent()
{
}

void VegetationComponent::Start()
{
}

Json::Object VegetationComponent::SaveComponent()
{

    m_model->SetPath(Editor::GetInstance()->GetAssetBrowser()->m_currentOpenFolder / m_model->m_filename);
    m_model->CalculateBoundingBox();
    ResourceManager::GetInstance()->AddResource<Model>(m_model);
    m_model->Save();

	Json::Object object;

	//object.emplace("Shape Type", (int)GetShapeType());

	return std::move(object);
}

void VegetationComponent::LoadComponent(const rapidjson::Value& object)
{
	//SetShape(static_cast<PhysXManager::PhysicsShape>(object["Shape Type"].GetInt()));
}

void VegetationComponent::Render()
{
    //if (m_instData.Buffer)
    //{
    //    m_instData.Buffer.Reset();
    //}
    //BillboardRenderer::AddBillboard(m_leafMaterial, BillboardRenderData{ *m_billboardVertices.get(), m_instData});

    //DrawSpline(m_spline, m, m_params.TrunkBottomWidth, m_params.TrunkTopWidth);
    //
    //for (auto& branch : m_branches)
    //{
    //    Math::Matrix b = Math::Matrix::MakeRotationY(branch.m_rotationInRads);
    //    b.SetPosition(m_spline.GetPositionAtFraction(branch.m_fraction));
    //
    //    b = b * Math::Matrix::MakeScale(Math::Vector3(branch.m_scale));
    //
    //    DrawSpline(branch.m_spline, b * m, m_params.BranchStartWidth * branch.m_scale, m_params.BranchEndWidth * branch.m_scale);
    //}
}

void VegetationComponent::Generate()
{
    m_model->m_root.m_mesh->vertexes.clear();
    m_model->m_root.m_mesh->indices32.clear();

    Spline baseSpline{};


    baseSpline.m_points.push_back(Math::Vector3(0.0f, 0.0f, 0.0f));
    baseSpline.m_points.push_back(Math::Vector3(0.0f+ Math::Random(-m_params.TrunkShapeRandomness, m_params.TrunkShapeRandomness), 2.0f+Math::Random(-m_params.TrunkShapeRandomness, m_params.TrunkShapeRandomness), 0.0f+Math::Random(-m_params.TrunkShapeRandomness, m_params.TrunkShapeRandomness)));
    baseSpline.m_points.push_back(Math::Vector3(0.0f+ Math::Random(-m_params.TrunkShapeRandomness, m_params.TrunkShapeRandomness), 4.0f+Math::Random(-m_params.TrunkShapeRandomness, m_params.TrunkShapeRandomness), 0.0f+Math::Random(-m_params.TrunkShapeRandomness, m_params.TrunkShapeRandomness)));
    baseSpline.m_points.push_back(Math::Vector3(0.0f+ Math::Random(-m_params.TrunkShapeRandomness, m_params.TrunkShapeRandomness), 6.0f+Math::Random(-m_params.TrunkShapeRandomness, m_params.TrunkShapeRandomness), 0.0f+Math::Random(-m_params.TrunkShapeRandomness, m_params.TrunkShapeRandomness)));
    
    if (m_model->m_billboardBuffer.Buffer)
    {
        m_model->m_billboardBuffer.Buffer.Reset();
    }

    m_model->m_billboardVertices.clear();

    AddSplineToMesh(baseSpline, Math::Matrix::MakeIdentity(), m_params.TrunkBottomWidth, m_params.TrunkTopWidth,m_params.Segments, m_params.PolyCount);

    
        
    for (int i = 0; i < m_params.NbBranches; i++)
    {
        float heightFraction = Math::Random(0.1f, 0.94f);

        Math::Vector3 startPos = baseSpline.GetPositionAtFraction(heightFraction);
        Math::Vector3 tangent  = baseSpline.GetTangentAtFraction(heightFraction).GetNormal();

        // Build orthonormal basis around tangent
        Math::Vector3 helper = fabs(tangent.y) < 0.99f ? Math::Vector3(0, 1, 0): Math::Vector3(1, 0, 0);

        Math::Vector3 normal = tangent.Cross(helper).GetNormal();
        Math::Vector3 binormal = tangent.Cross(normal).GetNormal();

        // Random direction around the trunk
        float angle = Math::Random(0.0f, Math::TwoPI);

        Math::Vector3 outward = normal * cos(angle) + binormal * sin(angle);
        outward += tangent*0.6f;
        outward.Normalize();

        float length = m_params.branchLength * 2.0f * (1.0f - heightFraction) * Math::Random(0.7f, 1.3f);

        int nbPoints = 5;

        Spline spline;
        spline.m_points.push_back(startPos);

        for (int i = 0; i < nbPoints; i++)
        {
            float t = static_cast<float>(i) / static_cast<float>(nbPoints);


            float step = length * t;

            float curve = t * t;
            outward.y += curve * m_params.branchUppWeight;
            spline.m_points.push_back(startPos + outward * step );
        }
        AddSplineToMesh(spline, Math::Matrix::MakeIdentity(), m_params.BranchStartWidth, m_params.BranchStartWidth * m_params.BranchWidthFalloff, 3, 4);

        AddBranch(spline, m_params.BranchStartWidth * m_params.BranchWidthFalloff, m_model->m_billboardVertices, false, heightFraction);
        AddBranch(spline, m_params.BranchStartWidth * m_params.BranchWidthFalloff, m_model->m_billboardVertices, true, heightFraction);
    }



    if (m_model->m_root.m_mesh->m_pVertexBuffer)
    {
        m_model->m_root.m_mesh->m_pVertexBuffer.Reset();
    }
    if (m_model->m_root.m_mesh->m_pIndexBuffer)
    {
        m_model->m_root.m_mesh->m_pIndexBuffer.Reset();
    }

    if (!m_model->m_billboardVertices.empty())
    {
        m_model->m_billboardBuffer.Buffer = Render::CreateVertexBuffer(Render::GetDevice(), m_model->m_billboardVertices);
        m_model->m_billboardBuffer.NbVertices = static_cast<uint32_t>(m_model->m_billboardVertices.size());
    }
    m_model->m_root.m_mesh->CreateVertexAndIndexBuffer(Render::GetDevice());
    m_model->m_root.m_mesh->CalculateBoundingBox();
}

void VegetationComponent::AddBranch(const Spline& parentSpline, float startWidth, std::vector<BillboardVertex>& leafs, bool flipped, float heigthFraction)
{
    if (startWidth < 0.01f)
    {
        if (flipped)
        {
            BillboardVertex leaf;
            leaf.Position = parentSpline.GetPositionAtFraction(1.0f);
            leaf.Size = m_params.leafSize * Math::Random(0.7f,1.3f);

            leafs.push_back(leaf);
        }
        return;
    }

    Math::Vector3 startPos = parentSpline.GetPositionAtFraction(1.0f);
    Math::Vector3 tangent = parentSpline.GetTangentAtFraction(0.9f).GetNormal();

    // Build orthonormal basis around tangent
    Math::Vector3 helper = fabs(tangent.y) < 0.99f ? Math::Vector3(0, 1, 0) : Math::Vector3(1, 0, 0);

    Math::Vector3 normal = tangent.Cross(helper).GetNormal();
    Math::Vector3 binormal = tangent.Cross(normal).GetNormal();

    // Random direction around the trunk
    //float angle = Math::Random(0.0f, Math::TwoPI);
    float angle = flipped ? Math::PI : 0.0f;

    angle += Math::Random(-m_params.BranchRandomDirection, m_params.BranchRandomDirection);


    Math::Vector3 outward = normal * cos(angle) + binormal * sin(angle);

    outward += tangent;
    outward.y += heigthFraction*0.3f;

    outward.Normalize();

    float length = m_params.branchLength * Math::Random(0.7f, 1.3f);


    Spline spline;
    spline.m_points.push_back(startPos);

    int nbPoints = 5;


    Math::Vector3 bendDirection(Math::Random(-1.0f, 1.0f), Math::Random(-1.0f, 1.0f), Math::Random(-1.0f, 1.0f));
    bendDirection.Normalize();
    float bendStrength = length * 0.5f;

    for (int i = 1; i <= nbPoints; i++)
    {
        float t = static_cast<float>(i) / static_cast<float>(nbPoints);

        // Main branch direction
        Math::Vector3 pos = startPos + outward * (length * t);

        // Curved bend
        float curve = t * t;

        pos += bendDirection * curve * bendStrength;

        spline.m_points.push_back(pos);
    }


    float endWidth = startWidth * m_params.BranchWidthFalloff;

    AddSplineToMesh(spline, Math::Matrix::MakeIdentity(), startWidth, endWidth,2,3);



    AddBranch(spline, endWidth,leafs, false, heigthFraction);
    AddBranch(spline, endWidth,leafs, true, heigthFraction);
}

void VegetationComponent::AddSplineToMesh(
    const Spline& spline,
    const Math::Matrix& localPose,
    float startWidth,
    float endWidth
    , int segments, int columns)
{
    Math::Vector3 previousPoint =
        spline.GetPositionAtFraction(0.0f);

    Math::Vector3 previousRight;
    Math::Vector3 forward;

    bool firstFrame = true;

    for (int i = 0; i < segments; i++)
    {
        float fraction =
            static_cast<float>(i) /
            static_cast<float>(segments - 1);

        Math::Vector3 point =
            spline.GetPositionAtFraction(fraction);

        if (i == 0)
        {
            forward =
                spline.GetPositionAtFraction(0.01f) - point;
            forward = forward.GetNormal();
        }
        else
        {
            forward =
                (point - previousPoint).GetNormal();
        }

        float radius =
            std::lerp(startWidth, endWidth, fraction);

        Math::Vector3 right;
        Math::Vector3 up;

        if (firstFrame)
        {
            Math::Vector3 worldUp(0, 1, 0);

            if (fabs(forward.Dot(worldUp)) > 0.99f)
                worldUp = Math::Vector3(1, 0, 0);

            right =
                worldUp.Cross(forward).GetNormal();

            up =
                forward.Cross(right).GetNormal();

            previousRight = right;
            firstFrame = false;
        }
        else
        {
            up =
                forward.Cross(previousRight).GetNormal();

            right =
                up.Cross(forward).GetNormal();

            previousRight = right;
        }

        int ringStart =
            static_cast<int>(m_model->m_root.m_mesh->vertexes.size());

        for (int j = 0; j < columns; ++j)
        {
            float angle =
                (static_cast<float>(j) / columns) *
                Math::TwoPI;

            float cs = cosf(angle);
            float sn = sn = sinf(angle);

            Math::Vector3 normal =
                (right * cs + up * sn).GetNormal();

            Math::Vector3 position =
                point + normal * radius;

            Mesh::Vertex vertex{};
            vertex.position = position.Transform(localPose);
            vertex.normal = normal.TransformNormal(localPose);
            vertex.tangent = forward.TransformNormal(localPose);
            vertex.uv = Math::Vector2(
                static_cast<float>(j) / (columns - 1),
                fraction * 6.0f);
            m_model->m_root.m_mesh->vertexes.push_back(vertex);
        }

        if (i > 0)
        {
            int currentStart = ringStart;
            int previousStart = ringStart - columns;

            for (int j = 0; j < columns; ++j)
            {
                int next = (j + 1) % columns;

                uint32_t i0 = currentStart + j;
                uint32_t i1 = currentStart + next;

                uint32_t i2 = previousStart + j;
                uint32_t i3 = previousStart + next;

                m_model->m_root.m_mesh->indices32.push_back(i0);
                m_model->m_root.m_mesh->indices32.push_back(i1);
                m_model->m_root.m_mesh->indices32.push_back(i2);

                m_model->m_root.m_mesh->indices32.push_back(i1);
                m_model->m_root.m_mesh->indices32.push_back(i3);
                m_model->m_root.m_mesh->indices32.push_back(i2);
            }
        }

        previousPoint = point;
    }
}

void VegetationComponent::DrawSpline(const Spline& spline, const Math::Matrix& matrix, float maxWidth, float minWidth)
{


    Math::Vector3 previousPoint;

    for (int i = 0; i < m_params.Segments; i++)
    {
        float fraction = static_cast<float>(i) / static_cast<float>(m_params.Segments - 1);
        Math::Vector3 point = spline.GetPositionAtFraction(fraction).Transform(matrix);
        if (i == 0)
        {
            previousPoint = point;
            continue;
        }
        Math::Vector3 forward = (previousPoint - point).GetNormal();
        float radius = std::lerp(maxWidth, minWidth, fraction);

        Math::Vector3 previousCirclePoint;
        Math::Vector3 worldUp(0.0f, 1.0f, 0.0f);

        if (fabs(forward.Dot(worldUp)) > 0.99f)
        {
            worldUp = Math::Vector3(1.0f, 0.0f, 0.0f);
        }

        Math::Vector3 right = worldUp.Cross(forward).GetNormal();

        Math::Vector3 up = forward.Cross(right).GetNormal();



        int polyCount = m_params.PolyCount;

        for (int j = 0; j <= polyCount; ++j)
        {
            float angle =(static_cast<float>(j) / polyCount)* 2.0f* 3.14159265f;

            float cs = cosf(angle);
            float sn = sinf(angle);

            Math::Vector3 offset = right * cs * radius +up * sn * radius;

            Math::Vector3 circlePoint = point + offset;

            if (j > 0)
            {
                Render::DrawLine(previousCirclePoint,circlePoint);
            }

            previousCirclePoint = circlePoint;
        }
        previousPoint = point;
    }
}





#ifdef EDITOR
void VegetationComponent::ComponentEditor()
{

    bool changed = false;
    {
        ImGui::Text("Trunk Material");
        if (m_model->GetMaterials()[0])
        {
            ImGui::Text(m_model->GetMaterials()[0]->GetFilename().c_str());
            ImGui::Image(ThumbnailManager::GetThumbnail(m_model->GetMaterials()[0].get())->GetSRV(), Editor::GetInstance()->GetDefaultTextureSize());

        }
        else
        {
            ImGui::Text("Default Material");
            ImGui::Image(ThumbnailManager::GetThumbnail(ResourceManager::GetInstance()->GetResource<DefaultMaterial>("Default.material").get())->GetSRV(), Editor::GetInstance()->GetDefaultTextureSize());
        }

        if (Editor::GetInstance()->GetDropResource(m_model->GetMaterials()[0]))
        {
            changed |= true;
        }
    }
    {
        ImGui::Text("Leaf Material");
        if (m_model->GetMaterials()[1])
        {
            ImGui::Text(m_model->GetMaterials()[1]->GetFilename().c_str());
            ImGui::Image(ThumbnailManager::GetThumbnail(m_model->GetMaterials()[1].get())->GetSRV(), Editor::GetInstance()->GetDefaultTextureSize());

        }
        else
        {
            ImGui::Text("Default Material");
            ImGui::Image(ThumbnailManager::GetThumbnail(ResourceManager::GetInstance()->GetResource<DefaultMaterial>("Default.material").get())->GetSRV(), Editor::GetInstance()->GetDefaultTextureSize());
        }

        if (Editor::GetInstance()->GetDropResource(m_model->GetMaterials()[1]))
        {
            changed |= true;
        }
    }
    ImGui::Separator();


    ImGui::SliderFloat("Trunk Bottom Width",&m_params.TrunkBottomWidth,0.1f, 10.0f);

    ImGui::SliderFloat("Trunk Top Width",&m_params.TrunkTopWidth,0.01f, 5.0f);

    ImGui::SliderFloat("Trunk Shape Randomness",&m_params.TrunkShapeRandomness,0.01f, 5.0f);

    ImGui::SliderInt("Number of Branches",&m_params.NbBranches,0, 200);

    ImGui::SliderFloat("branch Length",&m_params.branchLength,0.001f, 1.0f);
    ImGui::SliderFloat("Branch Start Width",&m_params.BranchStartWidth,0.001f, 1.0f);

    ImGui::SliderFloat("Branch Branch Width Falloff",&m_params.BranchWidthFalloff,0.001f, 0.95f);
    ImGui::SliderFloat("Branch Random Direction",&m_params.BranchRandomDirection,0.001f, 0.95f);
    ImGui::SliderFloat("branch Upp Weight",&m_params.branchUppWeight,0.001f, 2.95f);
    ImGui::SliderFloat("leaf Size",&m_params.leafSize,0.001f, 2.95f);

    ImGui::SliderInt("Poly Count",&m_params.PolyCount,0, 200);
    ImGui::SliderInt("Segment Count",&m_params.Segments,0, 200);

    if (ImGui::Button("Regenerate"))
    {
        Generate();
    }
}
#endif // EDITOR

Math::Vector3 Spline::GetPositionAtFraction(float fraction) const
{
    if (m_points.size() < 2)
    {
        return Math::Vector3(0.0f);
    }

    fraction = std::clamp(fraction, 0.000f, 1.0f);

    float wholePart = fraction * static_cast<float>(m_points.size()-1);
    int i = static_cast<int>(std::floor(fraction * static_cast<float>(m_points.size() - 1)));


    float fractionalPart = wholePart - static_cast<float>(i);

    size_t i0 = (i == 0) ? 0 : i - 1;
    size_t i1 = i;
    size_t i2 = i + 1;
    size_t i3 = (i + 2 >= m_points.size()) ? m_points.size() - 1 : i + 2;


    if (i == m_points.size() || i2 == m_points.size())
    {
        return m_points.back();
    }

    const auto p0 = m_points[i0];
    const auto p1 = m_points[i1];
    const auto p2 = m_points[i2];
    const auto p3 = m_points[i3];


    return CatmullRom(p0, p1, p2, p3, fractionalPart);
}

Math::Vector3 Spline::GetTangentAtFraction(float fraction) const
{
    fraction = std::clamp(fraction, 0.000f, 1.0f);

    float wholePart = fraction * static_cast<float>(m_points.size() - 1);
    int i = static_cast<int>(std::floor(fraction * static_cast<float>(m_points.size() - 1)));


    float fractionalPart = wholePart - static_cast<float>(i);

    size_t i0 = (i == 0) ? 0 : i - 1;
    size_t i1 = i;
    size_t i2 = i + 1;
    size_t i3 = (i + 2 >= m_points.size()) ? m_points.size() - 1 : i + 2;

    const auto p0 = m_points[i0];
    const auto p1 = m_points[i1];
    const auto p2 = m_points[i2];
    const auto p3 = m_points[i3];


    return CatmullRomTangent(p0, p1, p2, p3, fractionalPart);
}

Math::Vector3 Spline::CatmullRom(const Math::Vector3& p0, const Math::Vector3& p1, const Math::Vector3& p2, const Math::Vector3& p3, float t) const
{
    float t2 = t * t;
    float t3 = t2 * t;

    return 0.5f * (
        (2.0f * p1) +
        (-p0 + p2) * t +
        (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
        (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3
        );
}
Math::Vector3 Spline::CatmullRomTangent(
    const Math::Vector3& p0,
    const Math::Vector3& p1,
    const Math::Vector3& p2,
    const Math::Vector3& p3,
    float t) const
{
    float t2 = t * t;

    Math::Vector3 tangent =
        0.5f * (
            (-p0 + p2) +
            (2.0f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3)) * t +
            (3.0f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3)) * t2
            );

    return tangent.GetNormal();
}