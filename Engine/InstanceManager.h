#pragma once
#include <d3d11.h>
#include <unordered_map>
#include <wrl.h>

#include "Matrix.h"
#include "Mesh.h"
#include "Material.h"

struct DrawableInstance
{
	std::shared_ptr<Mesh> Mesh;
	std::shared_ptr<Material> Material;


	bool operator==(const DrawableInstance& other) const
	{
		return Mesh == other.Mesh && Material == other.Material;
	}
};

namespace std
{
	template<>
	struct hash<DrawableInstance>
	{
		size_t operator()(const DrawableInstance& instance) const
		{
			size_t h1 = std::hash<std::shared_ptr<Mesh>>{}(instance.Mesh);
			size_t h2 = std::hash<std::shared_ptr<Material>>{}(instance.Material);
			return h1 ^ (h2 << 1);
		}
	};
}


class InstanceManager
{
public:

	struct InstanceData
	{
		Microsoft::WRL::ComPtr<ID3D11Buffer> Buffer;
		uint32_t CurrentInstanceCount;
		uint32_t MaxInstanceCount;
	};


	void Map(ID3D11DeviceContext* context, ID3D11Device* device);
	void AddInstance(DrawableInstance drawable, const Math::Matrix& matrix);
	const std::unordered_map<DrawableInstance, InstanceData>& GetInstances() const;
private:

	void CreateInstanceBuffer(
		ID3D11Device* device, 
		const DrawableInstance& drawable,
		const std::vector<Math::Matrix>& matrices,
		uint32_t maxInstances
	);

	std::unordered_map<DrawableInstance, std::vector<Math::Matrix>> m_pendingInstances;
	std::unordered_map<DrawableInstance, InstanceData> m_instanceBuffers;
};

