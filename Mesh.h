#pragma once
#include "pch.h"
#include "Vector3.h"
#include <vector>
#include "Triangle.h"
#include "PhysXManager.h"
#include <string>
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "Texture.h"
#include <memory>
#include "Material.h"

struct aiNode;
struct aiMesh;
struct aiScene;

class Mesh
{
public:
	typedef struct Vertex {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT3 tangent;
	} VertexStruct;

	Mesh();

	void SetMaterial(std::shared_ptr<Material> material) { m_material = material; }
	std::shared_ptr<Material> GetMaterial() { return m_material; }

	void CalculateBoundingBox();

	PxTriangleMesh* CreatePhysxTriangleMesh() const;

	void SetName(const std::string& name);
	std::string& GetName();

	std::vector<Vertex> vertexes;
	std::vector<uint32_t> indices32;
	std::vector<Triangle> triangles;

	PxTriangleMesh* m_cachedTriangleMesh;

	Math::Vector3 m_maxBounds;
	Math::Vector3 m_minBounds;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer;

private:
	std::shared_ptr<Material> m_material;
	std::string m_meshName;
};

