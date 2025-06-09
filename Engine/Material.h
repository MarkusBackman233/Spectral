#pragma once
#include "pch.h"
#include "Vector4.h"
#include "LoadableResource.h"

class Texture;

class Material : public Resource
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
		bool LinearFiltering;
	};

	bool Load(const std::filesystem::path& file) override;

	Material();

	void SetTexture(int index, std::shared_ptr<Texture> texture);
	std::shared_ptr<Texture> GetTexture(int index) { return m_textures[index]; };


	MaterialSettings& GetMaterialSettings() { return m_settings; }

private:
	std::array<std::shared_ptr<Texture>, TextureType::NumTextures> m_textures{};

	MaterialSettings m_settings;


};

