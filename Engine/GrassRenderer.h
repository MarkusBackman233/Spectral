#pragma once
#include "Vector4.h"
#include <wrl/client.h>
#include "Matrix.h"

#include "TerrainComponent.h"

struct ID3D11Device;
struct ID3D11InputLayout;
struct ID3D11VertexShader;
struct ID3D11GeometryShader;
struct ID3D11PixelShader;
struct ID3D11DeviceContext;
struct ID3D11Buffer;

class DeviceResources;
class SkyboxManager;

class GrassRenderer
{
public:
	GrassRenderer() {};
	void CreateResources(ID3D11Device* device);

	static void AddGrassPatch(const GrassPatch& patch);

	void Render(ID3D11DeviceContext* context, const DeviceResources& deviceResources);
private:
	struct PixelConstantBuffer
	{
		Math::Vector4 data;
	};

	struct GeometryConstantBuffer
	{
		Math::Matrix viewProjection;
		Math::Vector4 cameraPos;
		float elapsedTime;
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


	static std::vector<GrassPatch> m_grassRenderQueue;
};

