#include "MeshComponent.h"
#include "Mesh.h"
#include "iRender.h"
#include "GameObject.h"
#include "Editor.h"
#include "PropertyWindowFactory.h"
#include "Texture.h"
#include "DefaultMaterial.h"
#include "ResourceManager.h"

MeshComponent::MeshComponent(GameObject* owner)
    : Component(owner)
    , m_mesh(nullptr)
{
    m_material = ResourceManager::GetInstance()->GetResource<DefaultMaterial>("Default.material");
}

MeshComponent::MeshComponent(GameObject* owner, MeshComponent* meshComponent)
    : Component(owner)
{
    SetMaterial(meshComponent->GetMaterial());
	SetMesh(meshComponent->GetMesh());
}

MeshComponent::MeshComponent(GameObject* owner, std::shared_ptr<Mesh> mesh)
    : Component(owner)
{
    m_material = ResourceManager::GetInstance()->GetResource<DefaultMaterial>("Default.material");
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
	    Render::DrawInstance(m_mesh, m_material, m_owner->GetWorldMatrix());
    }
}

void MeshComponent::Update(float deltaTime)
{

}

Json::Object MeshComponent::SaveComponent()
{
    Json::Object object;
    if (m_mesh)
    {
        object.emplace("Mesh", GetMesh()->GetFilename());
        object.emplace("Material", GetMaterial()->GetFilename());
    }
    else
    {
        object.emplace("Mesh", "NoName");
    }
    return std::move(object);
}

void MeshComponent::LoadComponent(const rapidjson::Value& object)
{
    std::string meshName = object["Mesh"].GetString();
    if (meshName != "NoName")
    {
        m_mesh = ResourceManager::GetInstance()->GetResource<Mesh>(meshName);
        m_material = ResourceManager::GetInstance()->GetResource<DefaultMaterial>(object["Material"].GetString());
    }
}

#ifdef EDITOR
void MeshComponent::ComponentEditor()
{
    std::string meshName = m_mesh ? m_mesh->GetFilename() : "Null";

    ImGui::Text(std::string("Mesh: " + meshName).c_str());
    if (ImGui::Button("Edit##Mesh"))
    {
        PropertyWindowFactory::SelectMesh(m_mesh);
    }
    ImGui::Separator();
    
    if (!m_mesh)
        return;

    ImGui::Text(std::string("Material: " + GetMaterial()->GetFilename()).c_str());
    if (ImGui::Button("Edit##Material"))
    {
        PropertyWindowFactory::SelectMaterial(m_material);
    }

    bool isDisabled = GetMaterial()->GetFilename() == "Default.material";

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

    static std::vector<std::pair<std::string, TextureType>> texturesNonCombined{
        {"Albedo", TextureType::Albedo},
        {"Normal", TextureType::Normal},
        {"Roughness", TextureType::Roughness},
        {"Metallic", TextureType::Metallic},
        {"Ao", TextureType::AmbientOcclusion},
    };
    static std::vector<std::pair<std::string, TextureType>> texturesCombined{
        {"Albedo", TextureType::Albedo},
        {"Normal", TextureType::Normal},
        {"MRO", TextureType::Roughness},
    };
    std::vector<std::pair<std::string, TextureType>>* textures = nullptr;


    if (m_material->GetMaterialSettings().CombinedMaterialTexture)
    {
        textures = &texturesCombined;
    }
    else
    {
        textures = &texturesNonCombined;
    }
    
    ImGui::Checkbox("Combined Material Texture", &GetMaterial()->GetMaterialSettings().CombinedMaterialTexture);

    for (const auto& [textureName, textureId] : *textures)
    {
        if (GetMaterial()->GetTexture(textureId) && GetMaterial()->GetTexture(textureId)->GetResourceView().Get())
        {
            auto selectedTextureName = GetMaterial()->GetTexture(textureId)->GetFilename();
            ImGui::Text(std::string(textureName + ": " + selectedTextureName).c_str());
            auto resource = GetMaterial()->GetTexture(textureId)->GetResourceView().Get();
            if (ImGui::ImageButton(textureName.c_str(),resource, Editor::GetInstance()->GetDefaultTextureSize()))
            {
                auto material = GetMaterial();
                PropertyWindowFactory::SelectTexture(material, textureId, selectedTextureName);
            }
        }
        else
        {
            ImGui::Text(std::string(textureName + ": Not selected").c_str());
    
            if (ImGui::Button(std::string("##"+ textureName).c_str(), Editor::GetInstance()->GetDefaultTextureSize()))
            {
                auto material = GetMaterial();
                PropertyWindowFactory::SelectTexture(material, textureId);
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
                    if (item.m_type != ResourceType::Texture)
                    {
                        continue;
                    }
                    auto droppedTexture = ResourceManager::GetInstance()->GetResource<Texture>(item.m_filename);

                    if (droppedTexture)
                    {
                        GetMaterial()->SetTexture(textureId, droppedTexture);
                    }
                }




            }
            ImGui::EndDragDropTarget();
        }

        ImGui::Separator();
    }

    ImGui::DragFloat("Roughness", &GetMaterial()->GetMaterialSettings().Roughness, 0.05f,0.0f,1.0f);
    ImGui::DragFloat("Metallic", &GetMaterial()->GetMaterialSettings().Metallic, 0.05f,0.0f,1.0f);
    ImGui::Checkbox("Backface Culling", &GetMaterial()->GetMaterialSettings().BackfaceCulling);
    ImGui::Checkbox("Linear Filtering", &GetMaterial()->GetMaterialSettings().LinearFiltering);
    ImGui::ColorPicker4("Color", &GetMaterial()->GetMaterialSettings().Color.x, Editor::GetInstance()->ColorPickerMask);

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

void MeshComponent::SetMesh(std::shared_ptr<Mesh> mesh)
{
	m_mesh = mesh;
}

void MeshComponent::SetMaterial(std::shared_ptr<DefaultMaterial> material)
{
    m_material = material;
}
