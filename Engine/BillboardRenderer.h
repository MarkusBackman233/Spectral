#pragma once
#include "Vector4.h"
#include <wrl/client.h>
#include "Matrix.h"
#include "InstanceManager.h"
struct ID3D11Device;
struct ID3D11InputLayout;
struct ID3D11VertexShader;
struct ID3D11GeometryShader;
struct ID3D11PixelShader;
struct ID3D11DeviceContext;
struct ID3D11Buffer;

class DeviceResources;
class SkyboxManager;
class DefaultMaterial;


struct BillboardVertex
{
	Math::Vector3 Position;
	float Size;
};

struct BillboardVertexData
{
	Microsoft::WRL::ComPtr<ID3D11Buffer> Buffer;
	uint32_t NbVertices;
};



struct BillboardRenderData
{
	BillboardVertexData m_vertexData;
	InstanceManager::InstanceData m_instanceData;
};

class BillboardRenderer
{
public:
	BillboardRenderer() {};
	void CreateResources(ID3D11Device* device);

	static void AddBillboard(std::shared_ptr<DefaultMaterial> Material, const BillboardRenderData& renderData);

	void Render(ID3D11DeviceContext* context, const DeviceResources& deviceResources);

private:
	struct GeometryConstantBuffer
	{
		Math::Matrix viewProjection;
		Math::Vector4 cameraPos;
		float elapsedTime;
		float unused1;
		float unused2;
		float unused3;
	};

	struct PixelConstantBuffer
	{
		Math::Vector3 cameraRight;
		Math::Vector3 cameraUp;
		Math::Vector3 cameraForward;
		float unused1;
		float unused2;
		float unused3;
	};

	struct MaterialGlobals
	{
		Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_pInputLayout;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_pVertexShader;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader>    m_pGeometryShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_pPixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pPixelConstantBufferData;
		Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pGeometryConstantBufferData;
		static_assert((sizeof(PixelConstantBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");

		PixelConstantBuffer m_pixelConstantBuffer;


		static_assert((sizeof(GeometryConstantBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");

		GeometryConstantBuffer m_geometryConstantBuffer;
	} m_materialGlobals;


	static std::unordered_map<std::shared_ptr<DefaultMaterial>, std::vector<BillboardRenderData>> m_renderQueue;
};

