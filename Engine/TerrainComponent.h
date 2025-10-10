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

	std::shared_ptr<Mesh> GetMesh() const;


#ifdef EDITOR
	 void ComponentEditor() override;
	 void DisplayComponentIcon() override;
#endif // EDITOR

	std::vector<GrassPatch> m_grassPatches;

	void GenerateGrass(GrassPatch& patch);

	void CreatePlaneMesh();
	void BuildTerrain();

	size_t GetVertexRowCount() const;
	float GetTerrainSize() const;

private:



	std::shared_ptr<Mesh> m_mesh;
	std::shared_ptr<TerrainMaterial> m_material;

	struct TerrainPointV1
	{
		float Height;
		uint32_t Color;
	};

	size_t m_vertexRowCount;
	float m_terrainSize;
	float m_uvScale;
};

