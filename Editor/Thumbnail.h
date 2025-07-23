#pragma once
#include <wrl/client.h>
#include <d3d11.h>
#include "Matrix.h"
#include "Vector4.h"
class Mesh;
class DefaultMaterial;

class Thumbnail
{
public:
	Thumbnail(Mesh* mesh, DefaultMaterial* material);
	~Thumbnail();
	ID3D11ShaderResourceView* GetSRV() { return m_SRV.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;

	Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_pInputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pVertexConstantBufferData;
	Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pPixelConstantBufferData;

	struct PixelConstantBuffer
	{
		Math::Vector4 data; // x = Shininess,  w = num lights
		Math::Vector4 data2; // x = hasBaseColor, y = hasNormal, z = linearFiltering, w = combined materials texture
		Math::Vector4 materialColor;
	};

	struct VertexConstantBuffer
	{
		Math::Matrix viewProjection;
		Math::Vector4 cameraPos;
	};

	static_assert((sizeof(VertexConstantBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");
	static_assert((sizeof(PixelConstantBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");

	VertexConstantBuffer m_vertexConstantBuffer;
	PixelConstantBuffer m_pixelConstantBuffer;
};

