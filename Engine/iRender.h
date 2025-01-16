#pragma once
#include "pch.h"
#include <stdexcept>
#include "DeviceResources.h"
#include "Vector4.h"
#include "Vector3.h"
#include "Vector2.h"
#include "Matrix.h"
#include "Camera.h"

typedef long HRESULT;
typedef unsigned int UINT;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11SamplerState;
struct ID3D11PixelShader;
struct ID3D11VertexShader;
struct D3D11_INPUT_ELEMENT_DESC;
struct ID3D11InputLayout;
struct ID3D11Buffer;
struct ID3D11Resource;
struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;
struct D3D11_BUFFER_DESC;
struct D3D11_SUBRESOURCE_DATA;

class Mesh;
class MeshComponent;
class LockedContext;
class Texture;

namespace Math
{
	class Vector2i;
	class Vector3;
	class Matrix;
};

namespace DirectX
{
	struct XMFLOAT4X4;
};


namespace Microsoft
{
	namespace WRL
	{
		template <typename T> class ComPtr;
	};
};

inline std::string HrToString(HRESULT hr)
{
	char s_str[64] = {};
	sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
	return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
	HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
	HRESULT Error() const { return m_hr; }
private:
	const HRESULT m_hr;
};

inline void ThrowIfFailed(HRESULT hr)
{
	if (((HRESULT)(hr)) < 0)
	{
		throw HrException(hr);
	}
}

namespace Render
{
	enum SHADER_TYPE : uint8_t
	{
		SHADER_TYPE_PIXEL,
		SHADER_TYPE_VERTEX,
	};

	ID3D11Device*			GetDevice();
	LockedContext	GetContext();
	ID3D11SamplerState*		GetDefaultSamplerState();

	void			DrawInstance(std::shared_ptr<Mesh> meshComponent, const Math::Matrix& matrix);

	const Math::Matrix&  GetViewMatrix();
	const Math::Matrix&  GetProjectionMatrix();
	const Math::Matrix& GetViewProjectionMatrix();
	Math::Vector3 GetCameraPosition();
	Math::Vector3 GetCameraDirection();
	Camera* GetCamera();

	std::vector<char>	LoadShaderBytecode(const std::string& filename);
	HRESULT				CreatePixelShader(ID3D11Device* device, const std::string& filename, ID3D11PixelShader** pixelShader);
	HRESULT				CreateVertexShader(ID3D11Device* device, const std::string& filename, ID3D11VertexShader** vertexShader, D3D11_INPUT_ELEMENT_DESC* inputDesc, size_t inputSize, ID3D11InputLayout** inputLayout);
	void				SetShaders(Microsoft::WRL::ComPtr<ID3D11PixelShader>& pixelShader, Microsoft::WRL::ComPtr<ID3D11VertexShader>& vertexShader, Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout, ID3D11DeviceContext* context);
	
	HRESULT CreateConstantBuffer(ID3D11Device* device, size_t size, Microsoft::WRL::ComPtr<ID3D11Buffer>& buffer);
	void	UpdateConstantBuffer(SHADER_TYPE shaderType, int slotIndex, Microsoft::WRL::ComPtr<ID3D11Buffer>& destinationData, const void* sourceData, ID3D11DeviceContext* context);


	void	DrawText2D(const std::string& text, const Math::Vector2& position);
	void	DrawLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Vector4& color = Math::Vector4(1,1,1,1));
	void	DrawGuizmo(const Math::Vector3& position, std::shared_ptr<Texture> texture, const Math::Vector4& color = Math::Vector4(1, 1, 1, 1));

	void CreateTexture(const void* textureData, Math::Vector2i size, Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture);

	Math::Vector2i	GetWindowSize();
	Math::Vector2	GetWindowSizef();
	void SetWindowSize(const Math::Vector2i& size);
	void SetWindowIcon(const std::string& iconName);
	void SetWindowTitle(const std::string& windowTitle);
	void ShowWindow(const bool show);
	void EnableDraggableBorders(const bool enable);

	HWND GetWindowHandle();
	HINSTANCE GetWinInstance();

	void CopyRenderTarget(ID3D11DeviceContext* context,ID3D11RenderTargetView* destination, ID3D11RenderTargetView* source);


	template<typename T>
	inline UINT GetVectorSize(const std::vector<T>& vector)
	{
		return static_cast<UINT>(sizeof(T) * vector.size());
	}

	template<typename T>
	Microsoft::WRL::ComPtr<ID3D11Buffer>	CreateIndexBuffer(ID3D11Device* device, const std::vector<T>& vector)
	{
		std::pair<D3D11_BUFFER_DESC, D3D11_SUBRESOURCE_DATA> bufferData{};
	
		bufferData.first.Usage = D3D11_USAGE_DEFAULT;
		bufferData.first.ByteWidth = GetVectorSize(vector);
		bufferData.first.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferData.first.CPUAccessFlags = 0;
		bufferData.first.MiscFlags = 0;
	
		ZeroMemory(&bufferData.second, sizeof(D3D11_SUBRESOURCE_DATA));
		bufferData.second.pSysMem = vector.data();
		bufferData.second.SysMemPitch = 0;
		bufferData.second.SysMemSlicePitch = 0;
		Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
		ThrowIfFailed(device->CreateBuffer(&bufferData.first, &bufferData.second, &indexBuffer));
		return indexBuffer;
	}
	
	template<typename T>
	Microsoft::WRL::ComPtr<ID3D11Buffer>	CreateVertexBuffer(ID3D11Device* device, const std::vector<T>& vector)
	{
		std::pair<D3D11_BUFFER_DESC, D3D11_SUBRESOURCE_DATA> bufferData{};
	
		bufferData.first.Usage = D3D11_USAGE_DEFAULT;
		bufferData.first.ByteWidth = GetVectorSize(vector);
		bufferData.first.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferData.first.CPUAccessFlags = 0;
		bufferData.first.MiscFlags = 0;
	
		ZeroMemory(&bufferData.second, sizeof(D3D11_SUBRESOURCE_DATA));
		bufferData.second.pSysMem = vector.data();
		bufferData.second.SysMemPitch = 0;
		bufferData.second.SysMemSlicePitch = 0;

		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
		ThrowIfFailed(device->CreateBuffer(&bufferData.first, &bufferData.second, &vertexBuffer));

		return vertexBuffer;
	}


	template<typename T>
	void MapVectorIntoBuffer(ID3D11DeviceContext* context, ID3D11Buffer* buffer, const std::vector<T>& vector)
	{
		D3D11_MAPPED_SUBRESOURCE resource{};
		ThrowIfFailed(context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource));
		memcpy(resource.pData, vector.data(), Render::GetVectorSize(vector));
		context->Unmap(buffer, 0);
	}
};
