#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "wrl.h"
#include "Material.h"

namespace Math
{
	class Vector3;
};

class LineMaterial : public Material
{
public:
	LineMaterial();

	void PreparePerMaterial() override;

	void SetColor(const Math::Vector3& color);
private:
	Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_pInputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pVertexConstantBufferData;
	Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pPixelConstantBufferData;

	struct PixelConstantBuffer {
		DirectX::XMFLOAT4 LineColor;
	};

	struct VertexConstantBuffer {
		DirectX::XMFLOAT4X4 viewProjection;
	};

	static_assert((sizeof(VertexConstantBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");
	static_assert((sizeof(PixelConstantBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");
	VertexConstantBuffer m_vertexConstantBuffer;
	PixelConstantBuffer m_pixelConstantBuffer;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer;
};

