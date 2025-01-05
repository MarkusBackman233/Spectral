#include "MeshComponent.h"
#include "ModelManager.h"
#include "Mesh.h"
#include "iRender.h"
#include <fstream>
#include <ppltasks.h>
#include "Logger.h"
#include "TextureManager.h"
#include "ObjectManager.h"
#include "IOManager.h"
#include "MaterialManager.h"
#include "StringUtils.h"
#include "Editor.h"
#include "PropertyWindowFactory.h"
#include "DirectXCollision.h"
#include "DxMathUtils.h"
#include "RenderManager.h"
#include "Editor.h"
#include "Texture.h"
#include "Material.h"

MeshComponent::MeshComponent(GameObject* owner)
    : Component(owner)
    , m_mesh(nullptr)
{

}

MeshComponent::MeshComponent(GameObject* owner, std::string filename)
    : Component(owner)
{
	SetMesh(filename);
}

MeshComponent::MeshComponent(GameObject* owner, MeshComponent* meshComponent)
    : Component(owner)
{
	SetMesh(meshComponent->GetMesh());
}

MeshComponent::MeshComponent(GameObject* owner, std::shared_ptr<Mesh> mesh)
    : Component(owner)
{
	SetMesh(mesh);
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
	    Render::DrawInstance(m_mesh, m_owner->GetWorldMatrix());
    }
}

void MeshComponent::Update(float deltaTime)
{

}

void MeshComponent::SaveComponent(rapidjson::Value& object, rapidjson::Document::AllocatorType& allocator)
{
    if (m_mesh)
    {
        object.AddMember("Mesh", rapidjson::Value(m_mesh->GetName().c_str(), allocator), allocator);
        object.AddMember("Material", rapidjson::Value(m_mesh->GetMaterial()->GetName().c_str(), allocator), allocator);
    }
    else
    {
        object.AddMember("Mesh", rapidjson::Value("NoName", allocator), allocator);
    }
}

void MeshComponent::LoadComponent(const rapidjson::Value& object)
{
    std::string meshName = object["Mesh"].GetString();
    if (meshName != "NoName")
    {
        m_mesh = ModelManager::GetInstance()->GetMesh(meshName);
        m_mesh->SetMaterial(MaterialManager::GetInstance()->GetMaterial(object["Material"].GetString()));
    }
}

#ifdef EDITOR
void MeshComponent::ComponentEditor()
{
    std::string meshName = m_mesh ? m_mesh->GetName() : "Null";

    ImGui::Text(std::string("Mesh: " + meshName).c_str());
    if (ImGui::Button("Edit##Mesh"))
    {
        PropertyWindowFactory::SelectMesh(m_mesh);
    }
    ImGui::Separator();
    
    if (!m_mesh)
        return;

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
            auto selectedTextureName = Editor::GetInstance()->GetObjectSelector()->SelectedGameObject()->GetComponentOfType<MeshComponent>()->GetMesh()->GetMaterial()->GetTexture(textureId)->GetFilename();
            ImGui::Text(std::string(textureName + ": " + selectedTextureName).c_str());
            auto resource = m_mesh->GetMaterial()->GetTexture(textureId)->GetResourceView().Get();
            if (ImGui::ImageButton(textureName.c_str(),resource, Editor::GetInstance()->GetDefaultTextureSize()))
            {
                auto material = m_mesh->GetMaterial();
                PropertyWindowFactory::SelectTexture(material, textureId, selectedTextureName);
            }
        }
        else
        {
            ImGui::Text(std::string(textureName + ": Not selected").c_str());
    
            if (ImGui::Button(std::string("##"+ textureName).c_str(), Editor::GetInstance()->GetDefaultTextureSize()))
            {
                auto material = m_mesh->GetMaterial();
                PropertyWindowFactory::SelectTexture(material, textureId);
            }
        }
        ImGui::Separator();
    }

    ImGui::DragFloat("Roughness", &m_mesh->GetMaterial()->GetMaterialSettings().Roughness, 0.05f,0.0f,1.0f);
    ImGui::DragFloat("Metallic", &m_mesh->GetMaterial()->GetMaterialSettings().Metallic, 0.05f,0.0f,1.0f);
    ImGui::Checkbox("Backface Culling", &m_mesh->GetMaterial()->GetMaterialSettings().BackfaceCulling);
    ImGui::ColorPicker4("Color", &m_mesh->GetMaterial()->GetMaterialSettings().Color.x, Editor::GetInstance()->ColorPickerMask);

    if (isDisabled)
    {
        ImGui::EndDisabled();
    }
    ImGui::Separator();
    if (ImGui::Button("Print Vertex Data"))
    {
        for (const auto& vertex : m_mesh->vertexes)
        {
            std::cout << "mesh->vertexes.push_back(Mesh::Vertex());" << std::endl;
            std::cout << "mesh->vertexes.back().position = DirectX::XMFLOAT3((float)"<< vertex.position.x <<", (float)" << vertex.position.y << ", (float)" << vertex.position.z << "); " << std::endl;
            std::cout << "mesh->vertexes.back().normal = DirectX::XMFLOAT3((float)"<< vertex.normal.x <<", (float)" << vertex.normal.y << ", (float)" << vertex.normal.z << "); " << std::endl;
            std::cout << "mesh->vertexes.back().tangent = DirectX::XMFLOAT3((float)"<< vertex.tangent.x <<", (float)" << vertex.tangent.y << ", (float)" << vertex.tangent.z << "); " << std::endl;
            std::cout << "mesh->vertexes.back().uv = DirectX::XMFLOAT2((float)"<< vertex.uv.x <<", (float)" << vertex.uv.y << "); " << std::endl;
        }

        
        std::cout << "mesh->indices32 = { ";
        for (const auto& index : m_mesh->indices32)
        {
            std::cout << index << ",";
        }
        std::cout<< " };" << std::endl;
    }

}
#endif
void MeshComponent::SetMesh(const std::string& filename)
{
	m_mesh = ModelManager::GetInstance()->GetMesh(filename);
}

void MeshComponent::SetMesh(std::shared_ptr<Mesh> mesh)
{
	m_mesh = mesh;
}
