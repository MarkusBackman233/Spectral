#pragma once
#include <d3d11.h>
#include <unordered_map>
#include <wrl.h>

#include "Matrix.h"
#include "Mesh.h"

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
	void AddInstance(std::shared_ptr<Mesh> mesh, const Math::Matrix& matrix);
	const std::unordered_map<std::shared_ptr<Mesh>, InstanceData>& GetInstances() const;
private:

	void CreateInstanceBuffer(
		ID3D11Device* device, 
		const std::shared_ptr<Mesh>& mesh, 
		const std::vector<Math::Matrix>& matrices,
		uint32_t maxInstances
	);

	std::unordered_map<std::shared_ptr<Mesh>, std::vector<Math::Matrix>> m_pendingInstances;
	std::unordered_map<std::shared_ptr<Mesh>, InstanceData> m_instanceBuffers;
};

