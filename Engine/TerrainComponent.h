#pragma once
#include "Component.h"
#include "pch.h"
#include <wrl/client.h>
#include <d3d11.h>
#include "Vector3.h"
#include "Vector2.h"

struct GrassVertexV1
{
	Math::Vector3 Position;
	Math::Vector2 Direction;
};

class Texture;

class GrassPatch
{
public:
	GrassPatch() {}
	~GrassPatch()
	{
		if (GrassPositionBufferData)
		{
			GrassPositionBufferData.Reset();
			GrassPositionBufferData = nullptr;
		}
	}


	Math::Vector3 BoundingMin; // Local to the terrain
	Math::Vector3 BoundingMax; 

	static constexpr float PatchSize = 10;
	unsigned int NumGrassPositions;
	std::vector<Math::Vector3> GrassTufts;
	Microsoft::WRL::ComPtr<ID3D11Buffer> GrassPositionBufferData = nullptr;
};

class Chunk
{
public:
	Chunk(uint16_t x, uint16_t z, const std::vector<float>& heightField, Math::Vector2 heightFieldTextureSize, size_t maxTerrainSize);
	Chunk() {};

	void Build();

	static constexpr size_t SizeInMeter = 128;
	static constexpr size_t Steps = 128;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_UAV;

	float m_minHeightBound;
	float m_maxHeightBound;


	uint16_t m_x;
	uint16_t m_z;
	std::vector<float> m_heights;
};

class Mesh;
class TerrainMaterial;

class TerrainComponent : public Component
{
public:
	TerrainComponent(GameObject* owner);
	Component::Type GetComponentType() override { return Component::Type::Terrain; };
	void Render() override;
	void Update(float deltaTime) override;

	Json::Object SaveComponent() override;
	void LoadComponent(const rapidjson::Value& object) override;


#ifdef EDITOR
	 void ComponentEditor() override;
	 void DisplayComponentIcon() override;
#endif // EDITOR

	std::vector<GrassPatch> m_grassPatches;

	void GenerateGrass(GrassPatch& patch);

	void CreateTerrain();

	float GetTerrainSize() const;

	std::shared_ptr<TerrainMaterial> m_material;

	std::shared_ptr<Texture> m_heightTexture;
	std::shared_ptr<Texture> m_worldTexture;

	float m_maxHight = 100.0f;

	std::vector<Math::Vector2> m_instances;
	Microsoft::WRL::ComPtr<ID3D11Buffer>      m_pInstanceBuffer;
	std::vector<Chunk> m_chunks;

	float m_terrainSize = 500.0f;

private:


	float m_terrainGenerationBlurRadius = 5.0f;
	float m_terrainGenerationBlurSigma = 2.5f;


	bool m_created = false;

	int m_resolution = 0;
};

