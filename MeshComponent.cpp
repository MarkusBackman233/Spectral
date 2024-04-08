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

MeshComponent::MeshComponent(GameObject* owner)
    : Component(owner)
{
    m_componentName = "MeshComponent";
    m_componentType = ComponentType_MeshComponent;
}

MeshComponent::MeshComponent(GameObject* owner, std::string filename)
    : Component(owner)
{
    m_componentName = "MeshComponent";
    m_componentType = ComponentType_MeshComponent;
	SetMesh(filename);
}

MeshComponent::MeshComponent(GameObject* owner, MeshComponent* meshComponent)
    : Component(owner)
{
    m_componentName = "MeshComponent";
    m_componentType = ComponentType_MeshComponent;
	SetMesh(meshComponent->GetMesh());
}

MeshComponent::MeshComponent(GameObject* owner, std::shared_ptr<Mesh> mesh)
    : Component(owner)
{
    m_componentName = "MeshComponent";
    m_componentType = ComponentType_MeshComponent;
	SetMesh(mesh);
}

void MeshComponent::Render()
{
    if(m_mesh.get() != nullptr)
    {
	    Render::DrawInstance(m_mesh, m_owner->GetMatrix());
    }
}

void MeshComponent::Update(float deltaTime)
{

}

void MeshComponent::SaveComponent(WriteObject& readObject)
{
    if (m_mesh)
    {
        readObject.Write(m_mesh->GetName());
    }
    else
    {
        readObject.Write("nomesh");
    }
}

void MeshComponent::LoadComponent(ReadObject& readObject)
{
    std::string mesh;
    readObject.Read(mesh);
    m_mesh = ModelManager::GetInstance()->GetMesh(mesh);
}

#ifdef EDITOR
void MeshComponent::ComponentEditor()
{
    std::string meshName = m_mesh ? m_mesh->GetName() : "Null";

    ImGui::Text(std::string("Mesh: " + meshName).c_str());
    if (ImGui::Button("Edit##Mesh"))
    {
        Editor::GetInstance()->OpenPropertyWindow(Editor::PropertyWindowType_Mesh);
    }
    ImGui::Separator();
    
    if (!m_mesh)
        return;

    ImGui::Text(std::string("Material: " + m_mesh->GetMaterial()->GetName()).c_str());
    if (ImGui::Button("Edit##Material"))
    {
        Editor::GetInstance()->OpenPropertyWindow(Editor::PropertyWindowType_Mesh);
    }
    

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
            ImGui::Text(std::string(texture.m_name + ": " + StringUtils::StripPathFromFilename(TextureManager::GetInstance()->GetTextureName(Editor::GetInstance()->SelectedGameObject()->GetComponentOfType<MeshComponent>()->GetMesh()->GetMaterial()->GetTexture(texture.m_textureId)))).c_str());
            auto resource = m_mesh->GetMaterial()->GetTexture(texture.m_textureId)->GetResourceView().Get();
            if (ImGui::ImageButton(resource, Editor::GetInstance()->GetDefaultTextureSize()) && m_mesh->GetMaterial()->GetName() != "default")
            {
                Editor::GetInstance()->OpenPropertyWindow(texture.m_propertyType);
            }
        }
        else
        {
            ImGui::Text(std::string(texture.m_name + ": Not selected").c_str());

            if (ImGui::Button(std::string("##"+ texture.m_name).c_str(), Editor::GetInstance()->GetDefaultTextureSize()))
            {
                Editor::GetInstance()->OpenPropertyWindow(texture.m_propertyType);
            }
        }
        ImGui::Separator();
    }

    ImGui::DragFloat("Roughness", &m_mesh->GetMaterial()->GetMaterialSettings().Roughness,1.0f,0.0f,1.0f);
    ImGui::DragFloat("Metallic", &m_mesh->GetMaterial()->GetMaterialSettings().Metallic,1.0f,0.0f,1.0f);

    if (ImGui::Button("Export Mesh"))
    {
        auto DropfileTask = Concurrency::create_task(
            [this]()
        {
            IOManager::SaveSpectralModel(m_mesh);
            IOManager::SaveSpectralMaterial(m_mesh->GetMaterial());
        }
        );
    }

    float uv[2] = { 1.0f, 1.0f };
    ImGui::PushItemWidth(208);
    ImGui::InputFloat2("Uv Scale##01", uv);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::SetCursorPosX(106);
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
