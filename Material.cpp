#include "Material.h"
#include <fstream>
#include "iRender.h"
#include <iostream>
#include "ShadowManager.h"
#include "Texture.h"
#include "SceneManager.h"
#include "RenderManager.h"

Material::Material()
{
    m_textures = { nullptr,nullptr,nullptr,nullptr,nullptr };
    m_materialSettings.BackfaceCulling = true;
    m_materialSettings.Roughness = 0.8f;
    m_materialSettings.Metallic = 0.0f;
    m_materialSettings.Color = Math::Vector4(1.0f,1.0f,1.0f,1.0f);
}


void Material::SetTexture(int index, std::shared_ptr<Texture> texture)
{
    m_textures[index] = texture;
}
