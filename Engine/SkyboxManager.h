#pragma once
#include <array>
#include <memory>
#include <vector>
#include "Vector4.h"
#include <wrl/client.h>
#include "Matrix.h"
class Mesh;
class DeviceResources;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;
struct ID3D11SamplerState;
struct ID3D11PixelShader;
struct ID3D11VertexShader;
struct ID3D11InputLayout;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11BlendState;
struct ID3D11Texture2D;

class SkyboxManager
{
public:
	class Cubemap
	{
	public:
		void SetMips(unsigned int numMips) 
		{ 
			RenderTargets.resize(numMips); 
			NumMips = numMips;
		}

		ID3D11ShaderResourceView* GetResource() const { return Resource.Get(); }
		ID3D11RenderTargetView* GetRenderTarget(int face, int mip = 0) const { return RenderTargets[mip][face].Get(); }

		Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Resource;
		std::vector<std::array<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>, 6>> RenderTargets;

		unsigned int Resolution;
		unsigned int NumMips;
	};


	SkyboxManager();

	void RenderSkybox(ID3D11DeviceContext* context, ID3D11RenderTargetView* renderTarget);
	void CreateResources(ID3D11DeviceContext* context, ID3D11Device* device);
	void DrawSkybox(ID3D11DeviceContext* context, ID3D11RenderTargetView* renderTarget);
	ID3D11SamplerState* GetCubeSamplerState() const { return m_cubeSamplerState.Get(); }
	void CreateCubeMap(ID3D11DeviceContext* context, ID3D11Device* device);

	ID3D11ShaderResourceView* GetSkyboxCubemap() const { return m_skybox.GetResource(); }
	ID3D11ShaderResourceView* GetIrradianceCubemap() const { return m_irradiance.GetResource(); }
	ID3D11ShaderResourceView* GetSpecularCubemap() const { return m_specular.GetResource(); }
private:

	Cubemap CreateCubemap(ID3D11DeviceContext* context, ID3D11Device* device, unsigned int resolution, unsigned int numMips, bool generateMips = false);

	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_cubeSamplerState;
	Microsoft::WRL::ComPtr<ID3D11BlendState>   m_alphaMixState;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_skyboxPixelShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_irradiancePixelShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_specularPixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_skyboxVertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_pInputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	   m_pPixelConstantBufferData;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	   m_pVertexConstantBufferData;

	Cubemap m_skybox;
	Cubemap m_specular;
	Cubemap m_lastSpecular;
	Cubemap m_irradiance;

	std::shared_ptr<Mesh> m_skyboxMesh;

	struct VertexConstantBuffer
	{
		Math::Matrix viewProjection;
		Math::Matrix lightMatrix;
		Math::Vector4 cameraPos;
	} m_vertexConstantBuffer;

	struct PixelConstantBuffer
	{
		Math::Vector4 skyboxColor; // w = use color as skybox
		Math::Vector4 data; 
	};
	static_assert((sizeof(PixelConstantBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");
	PixelConstantBuffer m_pixelConstantBuffer;
};

