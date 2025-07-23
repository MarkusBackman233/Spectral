#pragma once
#include "Component.h"
#include "pch.h"
#include <wrl/client.h>
#include <d3d11.h>
#include "Vector3.h"

struct GrassPatch
{

	Math::Vector3 BoundingMin; // Local to the terrain
	Math::Vector3 BoundingMax; 

	static constexpr float PatchSize = 10;
	unsigned int NumGrassPositions;
	Microsoft::WRL::ComPtr<ID3D11Buffer> GrassPositionBufferData;
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

	std::shared_ptr<Mesh> GetMesh();


#ifdef EDITOR
	 void ComponentEditor() override;
	 void DisplayComponentIcon() override;
#endif // EDITOR

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_splatTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_splatSRV;
private:
	void CreatePlaneMesh();
	std::shared_ptr<Mesh> m_mesh;
	std::shared_ptr<TerrainMaterial> m_material;

	std::vector<GrassPatch> m_grassPatches;


};

