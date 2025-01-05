#pragma once
#include "pch.h"
#include "Vector4.h"
#include "Texture.h"
class Material
{
public:


	enum TextureType
	{
		BaseColor,
		Normal,
		Roughness,
		Metallic,
		AmbientOcclusion,

		NumTextures,
	};

	struct MaterialSettings
	{
		float Roughness;
		float Metallic;
		Math::Vector4 Color;
		bool BackfaceCulling;
	};

	Material();

	void SetTexture(int index, std::shared_ptr<Texture> texture);
	std::shared_ptr<Texture> GetTexture(int index) { return m_textures[index]; };

	void SetName(const std::string& name) { m_name = name; }
	std::string& GetName() { return m_name; }

	MaterialSettings& GetMaterialSettings() { return m_materialSettings; }

private:
	std::array<std::shared_ptr<Texture>, TextureType::NumTextures> m_textures{};

	std::string m_name;
	MaterialSettings m_materialSettings;


};

