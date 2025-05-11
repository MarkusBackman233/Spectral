#include "Material.h"
#include "Texture.h"
#include "ResourceManager.h"
#include "Json.h"

Material::Material() : m_textures{}
{
    m_settings.BackfaceCulling = true;
    m_settings.LinearFiltering = true;
    m_settings.Roughness = 0.8f;
    m_settings.Metallic = 0.0f;
    m_settings.Color = Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}


bool Material::Load(const std::filesystem::path& file)
{
    auto json = Json::ParseFile(file);
    if (json.HasError())
    {
        SetTexture(0, ResourceManager::GetInstance()->GetResource<Texture>("TemplateGrid_albedo.bmp"));
        SetTexture(1, ResourceManager::GetInstance()->GetResource<Texture>("TemplateGrid_normal.bmp"));
        return true;
    }

    auto LoadTexture = [&](const char* textureName, Material::TextureType textureType)
    {
        if (json.Has(textureName))
        {
            std::string name = json[textureName].AsString();
            if (name == std::string("none") || name == std::string("None"))
            {
                return;
            }
            SetTexture(textureType, ResourceManager::GetInstance()->GetResource<Texture>(name));
        }
    };

    LoadTexture("Albedo", Material::BaseColor);
    LoadTexture("Normal", Material::Normal);
    LoadTexture("Roughness", Material::Roughness);
    LoadTexture("Metallic", Material::Metallic);
    LoadTexture("AmbientOcclusion", Material::AmbientOcclusion);

    if (json.Has("RougnessFloat"))
    {
        m_settings.Roughness = json["RougnessFloat"].AsFloat();
    }
    if (json.Has("MetallicFloat"))
    {
        m_settings.Metallic = json["MetallicFloat"].AsFloat();
    }
    if (json.Has("BackfaceCulling"))
    {
        m_settings.BackfaceCulling = json["BackfaceCulling"].AsBool();
    }
    if (json.Has("LinearFiltering"))
    {
        m_settings.LinearFiltering = json["LinearFiltering"].AsBool();
    }
    if (json.Has("Color")) 
    {
        auto colorObject = json["Color"].AsArray();
        auto& color = m_settings.Color;
        color.x = colorObject[0].AsFloat();
        color.y = colorObject[1].AsFloat();
        color.z = colorObject[2].AsFloat();
        color.w = colorObject[3].AsFloat();
    }
    return true;
}



void Material::SetTexture(int index, std::shared_ptr<Texture> texture)
{
    m_textures[index] = texture;
}
