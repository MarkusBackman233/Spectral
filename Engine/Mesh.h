#pragma once
#include "pch.h"
#include "Vector3.h"
#include "Vector2.h"
#include <d3d11.h>
#include <wrl/client.h>


class DefaultMaterial;

class Mesh
{
public:
	struct Vertex {
		Math::Vector3 position;
		uint32_t color;
		Math::Vector2 uv;
		Math::Vector3 normal;
		Math::Vector3 tangent;
	};

	Mesh(const std::string& name);

	const std::string& GetFilename();

	void CalculateBoundingBox();
	Math::Vector3 GetBoundingBoxMin() const { return m_minBounds; }
	Math::Vector3 GetBoundingBoxMax() const { return m_maxBounds; }

	void CreateVertexAndIndexBuffer(ID3D11Device* device);

	std::vector<Vertex> vertexes;
	std::vector<uint32_t> indices32;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer;

private:

	std::string m_name;

	Math::Vector3 m_maxBounds;
	Math::Vector3 m_minBounds;
};

