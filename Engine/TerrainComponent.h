#pragma once
#include "Component.h"
#include "pch.h"
#include <wrl/client.h>
#include <d3d11.h>
#include "Vector3.h"
#include "Vector2.h"
#include "InstanceManager.h"
#include "iRender.h"

namespace physx
{
	class PxRigidActor;
}

class VegetationComponent;
class Mesh;
class Model;
class TerrainMaterial;

class HeightmapProcessor
{
public:
	struct Settings
	{
		bool useGaussianBlur = false;
		int blurRadius = 3;
		float blurSigma = 1.0f;
	};

public:

	float GetHeight(size_t x, size_t z) const;

	HeightmapProcessor(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		ID3D11Texture2D* source, 
		size_t maxTerrainSize,
		const Settings& settings);
	UINT m_width;
	UINT m_height;
	float m_maxTerrainSize;

private:

	float ReadHeightPixel(uint8_t* row, UINT x, DXGI_FORMAT format, const Settings& settings);

	void GaussianBlur(const std::vector<float>& input,
		std::vector<float>& output,
		int width, int height,
		int radius, float sigma);

	std::vector<float> CreateKernel(int radius, float sigma);

	std::vector<float> m_heights;
};
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

	static constexpr float PatchSize = 128.0f;
	unsigned int NumGrassPositions;
	std::vector<Math::Vector3> GrassTufts;
	Microsoft::WRL::ComPtr<ID3D11Buffer> GrassPositionBufferData = nullptr;
};




struct ClutterInstance
{
	std::vector<Math::Matrix> m_poses;


	InstanceManager::InstanceData m_instanceData;
};


class Chunk
{
public:
	Chunk() {};
	void Feed(uint16_t x, uint16_t z, const HeightmapProcessor& heightField, const HeightmapProcessor& heightField2, float maxTerrainHeight);


	void Build();

	static constexpr size_t SizeInMeter = 128;
	static constexpr size_t Steps = 128;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_heightTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_heightSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_heightUAV;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_normalTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_normalSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_normalUAV;


	float m_minHeightBound;
	float m_maxHeightBound;


	uint16_t m_x;
	uint16_t m_z;
	std::vector<float> m_heights;
	std::vector<uint32_t> m_normals;

	std::unordered_map<std::shared_ptr<Model>, ClutterInstance> m_clutterInstances;

};


class TerrainComponent : public Component
{
public:
	TerrainComponent(GameObject* owner);
	~TerrainComponent();

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


	std::shared_ptr<VegetationComponent> m_vegetationComponent;

	float m_terrainSize = 500.0f;


	float GetHeightAtPosition(const Math::Vector3& Position) const;


	physx::PxRigidActor* GetActor();


private:
	physx::PxRigidActor* m_actor = nullptr;


	float m_terrainGenerationBlurRadius = 5.0f;
	float m_terrainGenerationBlurSigma = 2.5f;


	bool m_created = false;

	int m_resolution = 0;


};

