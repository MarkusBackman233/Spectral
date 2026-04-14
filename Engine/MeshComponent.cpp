#include "MeshComponent.h"
#include "Mesh.h"
#include "iRender.h"
#include "GameObject.h"
#include "Editor.h"
#include "PropertyWindowFactory.h"
#include "DefaultMaterial.h"
#include "ResourceManager.h"
#include "PhysXManager.h"
#ifdef EDITOR
#include <MaterialEditor.h>
#endif // EDITOR
#include <Thumbnail.h>
MeshComponent::MeshComponent(GameObject* owner)
    : Component(owner)
    , m_mesh(nullptr)
{
}

MeshComponent::MeshComponent(GameObject* owner, MeshComponent* meshComponent)
    : Component(owner)
{
	SetMesh(meshComponent->GetMesh());
    m_mass = meshComponent->m_mass;
    m_physicsType = meshComponent->m_physicsType;
}

MeshComponent::MeshComponent(GameObject* owner, std::shared_ptr<Model> mesh)
    : Component(owner)
{
	SetMesh(mesh);
}

MeshComponent::~MeshComponent()
{
    if (m_actor)
    {
        m_actor->release();
        m_actor = nullptr;
    }
}

void MeshComponent::Start()
{
    if (m_actor)
    {
        PxU32 numShapes = m_actor->getNbShapes();
        PxShape** shapes = new PxShape * [numShapes];
        m_actor->getShapes(shapes, numShapes);
        for (PxU32 i = 0; i < numShapes; i++)
        {
            m_actor->detachShape(*shapes[i]);
        }
        delete[] shapes;

        m_actor->release();
    }
    m_actor = PhysXManager::GetInstance()->CreateActor(static_cast<PhysXManager::PhysicsType>(m_physicsType), m_owner->GetWorldMatrix());
    m_actor->userData = m_owner;
    m_initialMatrix = m_owner->GetWorldMatrix();

    m_actor->setGlobalPose(PhysXManager::MatrixToPxTransform(m_owner->GetWorldMatrix()));
    PxFilterData filter;
    filter.word0 = 1;
    auto shape = PhysXManager::GetInstance()->CreateConvexTriangleShape(GetMesh(), GetOwner()->GetWorldMatrix().GetScale());
    shape->setQueryFilterData(filter);

    m_actor->attachShape(*shape);

    if (auto rigidActor = m_actor->is<PxRigidDynamic>())
    {
        PxRigidBodyExt::setMassAndUpdateInertia(*rigidActor, m_mass);
    }
}

void MeshComponent::Reset()
{
    if (m_physicsType == PhysXManager::PhysicsType::DynamicActor)
    {
        m_owner->SetWorldMatrix(m_initialMatrix);
    }
}

void MeshComponent::Render()
{
    if(m_mesh.get() != nullptr)
    {
        //DirectX::BoundingBox boundingBox;
        //DirectX::BoundingBox::CreateFromPoints(boundingBox, Spectral::DxMathUtils::ToDx(m_mesh->GetBoundingBoxMin()), Spectral::DxMathUtils::ToDx(m_mesh->GetBoundingBoxMax()));
        //boundingBox.Transform(boundingBox, Spectral::DxMathUtils::ToDx(m_owner->GetWorldMatrix()));
        //if (RenderManager::GetInstance()->GetFrustum().Contains(boundingBox))
        //{
        //}
        RenderSubMesh(m_mesh->m_root);
    }
}

void MeshComponent::RenderSubMesh(const SubMesh& subMesh)
{
    if (subMesh.m_mesh)
    {

        auto& material = m_mesh->GetMaterials()[subMesh.m_materialIndex];
        if (!material)
        {
            material = ResourceManager::GetInstance()->GetResource<DefaultMaterial>("Default.material");
        }

        Render::DrawInstance(subMesh.m_mesh, m_mesh->GetMaterials()[subMesh.m_materialIndex], subMesh.m_localMatrix * GetOwner()->GetWorldMatrix());
    }

    for (auto& subMesh : subMesh.m_submeshes)
    {
        RenderSubMesh(subMesh);
    }
}

bool MeshComponent::EditMaterial(SubMesh& subMesh)
{
    bool changed = false;


    for (auto& material : m_mesh->GetMaterials())
    {
        if (material)
        {
            ImGui::Text(material->GetFilename().c_str());
            ImGui::Image(ThumbnailManager::GetThumbnail(material.get())->GetSRV(), Editor::GetInstance()->GetDefaultTextureSize());

        }
        else
        {
            ImGui::Text("Default Material");
            ImGui::Image(ThumbnailManager::GetThumbnail(ResourceManager::GetInstance()->GetResource<DefaultMaterial>("Default.material").get())->GetSRV(), Editor::GetInstance()->GetDefaultTextureSize());
        }

        if (Editor::GetInstance()->GetDropResource(material))
        {
            changed |= true;
        }
        ImGui::Separator();
    }
    return changed;
}


void MeshComponent::Update(float deltaTime)
{
    if (m_actor && m_actor->is<PxRigidDynamic>())
    {
        if (!m_actor->is<PxRigidDynamic>()->isSleeping())
        {
            Math::Vector3 scale = m_owner->GetWorldMatrix().GetScale(); // this might cause object to shrink or grow overtime due to floating point precision
            PxMat44 pxMatrix = m_actor->getGlobalPose();
            Math::Matrix matrix = *static_cast<Math::Matrix*>((void*)&pxMatrix);
            m_owner->SetWorldMatrix(Math::Matrix::MakeScale(scale) * matrix);
        }
    }
}

Json::Object MeshComponent::SaveComponent()
{
    Json::Object object;
    if (m_mesh)
    {
        object.emplace("Mesh", GetMesh()->GetFilename());
    }
    else
    {
        object.emplace("Mesh", "NoName");
    }

    object.emplace("Physics Type", m_physicsType);
    object.emplace("Mass", m_mass);
    

    return std::move(object);
}

void MeshComponent::LoadComponent(const rapidjson::Value& object)
{
    std::string meshName = object["Mesh"].GetString();
    if (meshName != "NoName")
    {
        m_mesh = ResourceManager::GetInstance()->GetResource<Model>(meshName);
    }
    if (object.HasMember("Physics Type"))
    {
        SetPhysicsType(static_cast<PhysXManager::PhysicsType>(object["Physics Type"].GetInt()));
    }
}

#ifdef EDITOR
void MeshComponent::ComponentEditor()
{
    std::string meshName = m_mesh ? m_mesh->GetFilename() : "Null";

    ImGui::Text(std::string("Mesh: " + meshName).c_str());

    bool changed = false;

    if (m_mesh)
    {
        ImGui::Text(m_mesh->GetFilename().c_str());
        ImGui::Image(ThumbnailManager::GetThumbnail(m_mesh.get())->GetSRV(), Editor::GetInstance()->GetDefaultTextureSize());
    }
    else
    {
        ImGui::Text("Null Mesh");
        //ImGui::Image(ResourceManager::GetInstance()->GetResource<Texture>("TemplateGrid_albedo.bmp").get(), Editor::GetInstance()->GetDefaultTextureSize());
    }

    if (Editor::GetInstance()->GetDropResource(m_mesh))
    {
        changed |= true;
    }

    ImGui::Separator();
    
    if (!m_mesh)
        return;

    changed |= EditMaterial(m_mesh->m_root);

    if (changed)
    {
        m_mesh->SetUnsaved();
        ThumbnailManager::RegenerateThumbnail(m_mesh.get());
    }
    static  std::unordered_map<PhysXManager::PhysicsType, std::string> rigidbodyTypes =
    { {PhysXManager::PhysicsType::StaticActor, "Static"},
     {PhysXManager::PhysicsType::DynamicActor, "Dynamic"},
     {PhysXManager::PhysicsType::VehicleActor, "Vehicle"} };

    if (ImGui::BeginCombo("Rigidbody Type", rigidbodyTypes.find(static_cast<PhysXManager::PhysicsType>(m_physicsType))->second.c_str()))
    {
        ImGui::Separator();
        for (const auto& [rigidbodyType, name] : rigidbodyTypes)
        {
            bool isSelected = rigidbodyType == m_physicsType;
            if (ImGui::Selectable(name.c_str(), isSelected))
            {
                SetPhysicsType(rigidbodyType);
            }
        }
        ImGui::EndCombo();
    }
    if (m_actor)
    {
        auto rigidDynamic = m_actor->is<PxRigidDynamic>();

        if (rigidDynamic)
        {
            float mass = rigidDynamic->getMass();
            ImGui::PushItemWidth(104);
            if (ImGui::DragFloat("Mass", &mass))
            {
                rigidDynamic->setMass(mass);
                m_mass = mass;
            }
            ImGui::PopItemWidth();
            auto rigidbodyFlags = rigidDynamic->getRigidBodyFlags();
            bool isKinematic = rigidbodyFlags & PxRigidBodyFlag::eKINEMATIC;

            if (ImGui::Checkbox("Kinematic", &isKinematic))
            {
                rigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, isKinematic);
            }
        }
    }

}
#endif

void MeshComponent::SetMesh(std::shared_ptr<Model> mesh)
{
	m_mesh = mesh;
}

void MeshComponent::SetPhysicsType(int physicsType)
{
    m_physicsType = physicsType;
}