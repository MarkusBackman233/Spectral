#include "iRender.h"
#include "RenderManager.h"
#include <fstream>
#include "MeshComponent.h"

namespace Render
{
	ID3D11Device* GetDevice()
	{
		return RenderManager::GetInstance()->GetDeviceResources()->GetDevice();
	}
	LockedContext GetContext()
	{
		return RenderManager::GetInstance()->GetDeviceResources()->GetLockedDeviceContext();
	}	
	ID3D11SamplerState* GetDefaultSamplerState()
	{
		return RenderManager::GetInstance()->GetDeviceResources()->GetDefaultSamplerState();
	}

	void DrawInstance(std::shared_ptr<Mesh> mesh, Math::Matrix& matrix)
	{
		RenderManager::GetInstance()->DrawInstancedMesh(mesh, matrix);
	}

	Math::Matrix GetViewMatrix()
	{
		return RenderManager::GetInstance()->GetViewMatrix();
	}

	Math::Matrix GetProjectionMatrix()
	{
		return RenderManager::GetInstance()->GetProjectionMatrix();
	}
	const Math::Vector3& GetCameraPosition()
	{
		return RenderManager::GetInstance()->GetPlayer()->GetPosition();
	}
	DirectX::XMFLOAT4X4& GetViewProjectionMatrix()
	{
		return RenderManager::GetInstance()->GetViewProjectionMatrix();
	}

	std::vector<char> LoadShaderBytecode(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<char> buffer(size);
		file.read(buffer.data(), size);
		return buffer;
	}

	HRESULT CreatePixelShader(const std::string& filename, ID3D11PixelShader** pixelShader)
	{
		HRESULT hr = S_OK;
		auto buffer = LoadShaderBytecode(IOManager::ExecutableDirectory + filename);
		hr = Render::GetDevice()->CreatePixelShader(buffer.data(), buffer.size(), nullptr, pixelShader);
		ThrowIfFailed(hr);
		return hr;
	}
	HRESULT CreateVertexShader(const std::string& filename, ID3D11VertexShader** vertexShader, D3D11_INPUT_ELEMENT_DESC* inputDesc, size_t inputSize, ID3D11InputLayout** inputLayout)
	{
		HRESULT hr = S_OK;
		auto buffer = LoadShaderBytecode(IOManager::ExecutableDirectory + filename);
		hr = Render::GetDevice()->CreateVertexShader(buffer.data(), buffer.size(), nullptr, vertexShader);
		hr = Render::GetDevice()->CreateInputLayout(inputDesc, (UINT)inputSize, buffer.data(), buffer.size(), inputLayout);
		ThrowIfFailed(hr);
		return hr;
	}
	void SetShaders(Microsoft::WRL::ComPtr<ID3D11PixelShader>& pixelShader, Microsoft::WRL::ComPtr<ID3D11VertexShader>& vertexShader, Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
	{
		auto lockedContext = Render::GetContext();
		lockedContext.GetContext()->PSSetShader(pixelShader.Get(), nullptr, 0);
		lockedContext.GetContext()->VSSetShader(vertexShader.Get(), nullptr, 0);
		lockedContext.GetContext()->IASetInputLayout(inputLayout.Get());
	}


	HRESULT CreateConstantBuffer(size_t size, Microsoft::WRL::ComPtr<ID3D11Buffer>& buffer)
	{
		HRESULT hr = S_OK;
		CD3D11_BUFFER_DESC bufferDesc((UINT)size, D3D11_BIND_CONSTANT_BUFFER);
		hr = Render::GetDevice()->CreateBuffer(&bufferDesc, nullptr, &buffer);
		ThrowIfFailed(hr);
		return hr;
	}

	void UpdateConstantBuffer(SHADER_TYPE shaderType, int slotIndex, Microsoft::WRL::ComPtr<ID3D11Buffer>& destinationData, const void* sourceData)
	{
		auto lockedContext = Render::GetContext();
		lockedContext.GetContext()->UpdateSubresource(destinationData.Get(), 0, nullptr, sourceData, 0, 0);
		switch (shaderType)
		{
		case Render::SHADER_TYPE::SHADER_TYPE_PIXEL:
			lockedContext.GetContext()->PSSetConstantBuffers(slotIndex, 1, destinationData.GetAddressOf());
			break;
		case Render::SHADER_TYPE::SHADER_TYPE_VERTEX:
			lockedContext.GetContext()->VSSetConstantBuffers(slotIndex, 1, destinationData.GetAddressOf());
			break;
		}

	}


	void CreateVertexAndIndexBuffer(Mesh* mesh)
	{
		RenderManager::GetInstance()->CreateVertexAndIndexBuffer(mesh);
	}

	void CreateTexture(const void* textureData, Math::Vector2i size, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& resourceView, Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture)
	{
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = size.x;
		texDesc.Height = size.y;
		texDesc.MipLevels = texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = textureData;
		initData.SysMemPitch = size.x * 4; // 4 bytes per pixel for RGBA format
		ThrowIfFailed(Render::GetDevice()->CreateTexture2D(&texDesc, &initData, &texture));

		//D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		//srvDesc.Format = texDesc.Format;
		//srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		//srvDesc.Texture2D.MipLevels = 1;
		//srvDesc.Texture2D.MostDetailedMip = 0;
		//hr = Render::GetDevice()->CreateShaderResourceView(texture.Get(), &srvDesc, &resourceView); // Not needed since we are generating mips in another step.
	}

	void DrawLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Vector3& color)
	{
		RenderManager::GetInstance()->DrawLine(start, end, color);
	}

	Math::Vector2i GetWindowSize()
	{
		return RenderManager::GetInstance()->WindowSize;
	}

};
