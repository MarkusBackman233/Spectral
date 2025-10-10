#pragma once
#include <wrl/client.h>
#include <d3d11.h>
#include "Matrix.h"
#include "Vector4.h"
#include <unordered_map>
#include <memory>

struct ID3D11Device;

class Mesh;
class DefaultMaterial;


class Thumbnail
{
public:
	Thumbnail(Mesh* mesh, DefaultMaterial* material);
	~Thumbnail();

	static void CreateResources(ID3D11Device* device);
	static void DestroyResources();

	void Render(Mesh* mesh, DefaultMaterial* material);

	ID3D11ShaderResourceView* GetSRV() { return m_SRV.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;

	static struct GlobalThumbnailResources
	{
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
	} m_resources;

};

class ThumbnailManager
{
public:
	static std::shared_ptr<Thumbnail> GetThumbnail(DefaultMaterial* material);
	static void RegenerateThumbnail(DefaultMaterial* material);


private:
	static std::unordered_map<std::string, std::shared_ptr<Thumbnail>> m_thumbnails;
};