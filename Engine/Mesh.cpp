#include "Mesh.h"
#include "iRender.h"
#include "IOManager.h"

Mesh::Mesh(const std::string& name)
	: m_name(name)
{
}

const std::string& Mesh::GetFilename()
{
	return m_name;
}

void Mesh::CreateVertexAndIndexBuffer(ID3D11Device* device)
{
	m_pVertexBuffer = Render::CreateVertexBuffer(device, vertexes);
	m_pIndexBuffer = Render::CreateIndexBuffer(device, indices32);
}

void Mesh::CalculateBoundingBox()
{
	m_minBounds = Math::Vector3(std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max());
	m_maxBounds = Math::Vector3(std::numeric_limits<float>::min(),std::numeric_limits<float>::min(),std::numeric_limits<float>::min());

	for (const auto& vertex : vertexes)
	{
		m_minBounds.x = std::min(m_minBounds.x, vertex.position.x);
		m_minBounds.y = std::min(m_minBounds.y, vertex.position.y);
		m_minBounds.z = std::min(m_minBounds.z, vertex.position.z);
		m_maxBounds.x = std::max(m_maxBounds.x, vertex.position.x);
		m_maxBounds.y = std::max(m_maxBounds.y, vertex.position.y);
		m_maxBounds.z = std::max(m_maxBounds.z, vertex.position.z);
	}
}

