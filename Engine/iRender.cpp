#include "iRender.h"
#include "RenderManager.h"
#include <fstream>
#include "IOManager.h"
#include "GuiManager.h"
#include "InstanceManager.h"

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

	void DrawInstance(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, const Math::Matrix& matrix)
	{
		RenderManager::GetInstance()->GetInstanceManager()->AddInstance(DrawableInstance{ mesh,material }, matrix);
	}

	const Math::Matrix& GetViewMatrix()
	{
		return RenderManager::GetInstance()->GetCamera()->GetViewMatrix();
	}
	const Math::Matrix& GetProjectionMatrix()
	{
		return RenderManager::GetInstance()->GetCamera()->GetProjectionMatrix();
	}
	const Math::Matrix& GetViewProjectionMatrix()
	{
		return RenderManager::GetInstance()->GetCamera()->GetViewProjectionMatrix();
	}
	Math::Vector3 GetCameraPosition()
	{
		return RenderManager::GetInstance()->GetCamera()->GetWorldMatrix().GetPosition();
	}
	Math::Vector3 GetCameraDirection()
	{
		return -RenderManager::GetInstance()->GetCamera()->GetWorldMatrix().GetFront();
	}


	Camera* GetCamera()
	{
		return RenderManager::GetInstance()->GetCamera();
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

	HRESULT CreatePixelShader(ID3D11Device* device, const std::string& filename, ID3D11PixelShader** pixelShader)
	{
		HRESULT hr = S_OK;
		auto buffer = LoadShaderBytecode((IOManager::ExecutableDirectory / "shaders" / filename).string());
		hr = device->CreatePixelShader(buffer.data(), buffer.size(), nullptr, pixelShader);

		ThrowIfFailed(hr);
		return hr;
	}
	HRESULT CreateVertexShader(ID3D11Device* device, const std::string& filename, ID3D11VertexShader** vertexShader, D3D11_INPUT_ELEMENT_DESC* inputDesc, size_t inputSize, ID3D11InputLayout** inputLayout)
	{
		HRESULT hr = S_OK;
		auto buffer = LoadShaderBytecode((IOManager::ExecutableDirectory / "shaders" / filename).string());
		hr = device->CreateVertexShader(buffer.data(), buffer.size(), nullptr, vertexShader);
		hr = device->CreateInputLayout(inputDesc, (UINT)inputSize, buffer.data(), buffer.size(), inputLayout);
		ThrowIfFailed(hr);
		return hr;
	}
	void SetShaders(Microsoft::WRL::ComPtr<ID3D11PixelShader>& pixelShader, Microsoft::WRL::ComPtr<ID3D11VertexShader>& vertexShader, Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout, ID3D11DeviceContext* context)
	{
		context->PSSetShader(pixelShader.Get(), nullptr, 0);
		context->VSSetShader(vertexShader.Get(), nullptr, 0);
		context->IASetInputLayout(inputLayout.Get());
	}


	HRESULT CreateConstantBuffer(ID3D11Device* device, size_t size, Microsoft::WRL::ComPtr<ID3D11Buffer>& buffer)
	{
		HRESULT hr = S_OK;
		CD3D11_BUFFER_DESC bufferDesc((UINT)size, D3D11_BIND_CONSTANT_BUFFER);
		hr = device->CreateBuffer(&bufferDesc, nullptr, &buffer);
		ThrowIfFailed(hr);
		return hr;
	}

	void UpdateConstantBuffer(SHADER_TYPE shaderType, int slotIndex, Microsoft::WRL::ComPtr<ID3D11Buffer>& destinationData, const void* sourceData, ID3D11DeviceContext* context)
	{
		context->UpdateSubresource(destinationData.Get(), 0, nullptr, sourceData, 0, 0);
		switch (shaderType)
		{
		case SHADER_TYPE::SHADER_TYPE_PIXEL:
			context->PSSetConstantBuffers(slotIndex, 1, destinationData.GetAddressOf());
			break;
		case SHADER_TYPE::SHADER_TYPE_VERTEX:
			context->VSSetConstantBuffers(slotIndex, 1, destinationData.GetAddressOf());
			break;
		}

	}

	void CreateTexture(const void* textureData, Math::Vector2i size, Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture)
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
		initData.SysMemPitch = size.x * 4;
		ThrowIfFailed(Render::GetDevice()->CreateTexture2D(&texDesc, &initData, &texture));
	}

	void DrawLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Vector4& color)
	{
		RenderManager::GetInstance()->GetLineRenderer()->AddLine(start, end, color);
	}

	void DrawText2D(const std::string& text, const Math::Vector2& position)
	{
		RenderManager::GetInstance()->GetGuiManager()->Text(text, position);
	}

	void DrawGuizmo(const Math::Vector3& position, std::shared_ptr<Texture> texture, const Math::Vector4& color)
	{
		RenderManager::GetInstance()->GetGuizmoRenderer()->AddGuizmo(texture, position, color);
	}

	void DrawLight(const Light* light)
	{
		RenderManager::GetInstance()->GetPbrRenderer()->RenderLight(light);
	}

	Math::Vector2i GetWindowSize()
	{
		Math::Vector2 windowSize = RenderManager::GetInstance()->GetWindowsManager()->GetWindowSize();

		return Math::Vector2i(static_cast<int>(windowSize.x), static_cast<int>(windowSize.y));
	}

	Math::Vector2 GetWindowSizef()
	{
		return RenderManager::GetInstance()->GetWindowsManager()->GetWindowSize();
	}

	void SetWindowSize(const Math::Vector2i& size) 
	{
		RECT desktop;
		GetWindowRect(GetDesktopWindow(), &desktop);

		MoveWindow(Render::GetWindowHandle(),
			desktop.right / 2 - size.x / 2,
			desktop.bottom / 2 - size.y / 2,
			size.x,
			size.y,
			TRUE
		);
	}

	void SetWindowIcon(const std::string& iconName)
	{
		auto windowHandle = Render::GetWindowHandle();

		HANDLE hIcon = LoadImageA(0, iconName.c_str(), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
		SendMessage(windowHandle, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SendMessage(windowHandle, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	}

	void SetWindowTitle(const std::string& windowTitle)
	{
		SetWindowTextA(Render::GetWindowHandle(), windowTitle.c_str());

	}

	void ShowWindow(const bool show)
	{
		ShowWindow(Render::GetWindowHandle(), show ? SW_SHOW : SW_HIDE);
	}

	void EnableDraggableBorders(const bool enable)
	{
		if (enable)
		{
			::SetWindowLong(Render::GetWindowHandle(), GWL_STYLE, GetWindowLong(Render::GetWindowHandle(), GWL_STYLE) | WS_SIZEBOX);
		}
		else
		{
			::SetWindowLong(Render::GetWindowHandle(), GWL_STYLE, GetWindowLong(Render::GetWindowHandle(), GWL_STYLE) & ~WS_SIZEBOX);
		}
	}
	HWND GetWindowHandle()
	{
		return RenderManager::GetInstance()->GetWindowsManager()->GetWindowHandle();
	}
	HINSTANCE GetWinInstance()
	{
		return RenderManager::GetInstance()->GetWindowsManager()->GetWinInstance();
	}

	void CopyRenderTarget(ID3D11DeviceContext* context, ID3D11RenderTargetView* destination, ID3D11RenderTargetView* source)
	{
		ID3D11Resource* pSrcResource = nullptr;
		ID3D11Resource* pDstResource = nullptr;
		destination->GetResource(&pDstResource);
		source->GetResource(&pSrcResource);
		context->CopyResource(pDstResource, pSrcResource);
		pSrcResource->Release();
		pDstResource->Release();
	}

	/*
	Math::Vector2 RenderManager::WorldspaceToScreenspace(const Math::Vector3& worldspace) const
	{
		DirectX::XMMATRIX viewProjMatrix = DirectX::XMMatrixMultiply(m_viewMatrix, m_projectionMatrix);
		DirectX::XMVECTOR worldCoords = DirectX::XMVectorSet(worldspace.x, worldspace.y, worldspace.z, 1.0f);
		DirectX::XMVECTOR clipCoords = DirectX::XMVector4Transform(worldCoords, viewProjMatrix);

		DirectX::XMFLOAT4 clipCoordsFloat;
		DirectX::XMStoreFloat4(&clipCoordsFloat, clipCoords);

		if (clipCoordsFloat.w == 0.0f) {
			return Math::Vector2(100, 100);
		}

		DirectX::XMVECTOR ndcCoords = DirectX::XMVectorDivide(clipCoords, DirectX::XMVectorSplatW(clipCoords));
		Math::Vector2 screenCoords;
		DirectX::XMFLOAT4 ndcCoordsFloat;
		DirectX::XMStoreFloat4(&ndcCoordsFloat, ndcCoords);
		screenCoords.x = (ndcCoordsFloat.x + 1.0f) * 0.5f * Render::GetWindowSize().x;
		screenCoords.y = (1.0f - ndcCoordsFloat.y) * 0.5f * Render::GetWindowSize().y;
		return screenCoords;
	}
	*/
};
