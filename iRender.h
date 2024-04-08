#pragma once
#include "pch.h"
#include <stdexcept>

#include "DeviceResources.h"

#include "Vector3.h"

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

namespace Render
{
	enum SHADER_TYPE
	{
		SHADER_TYPE_PIXEL,
		SHADER_TYPE_VERTEX,
	};

	ID3D11Device*			GetDevice();
	LockedContext	GetContext();
	ID3D11SamplerState*		GetDefaultSamplerState();

	void			DrawInstance(std::shared_ptr<Mesh> meshComponent, Math::Matrix& matrix);

	Math::Matrix	GetViewMatrix();
	Math::Matrix	GetProjectionMatrix();
	const Math::Vector3& GetCameraPosition();
	DirectX::XMFLOAT4X4& GetViewProjectionMatrix();

	std::vector<char>	LoadShaderBytecode(const std::string& filename);
	HRESULT				CreatePixelShader(const std::string& filename, ID3D11PixelShader** pixelShader);
	HRESULT				CreateVertexShader(const std::string& filename, ID3D11VertexShader** vertexShader, D3D11_INPUT_ELEMENT_DESC* inputDesc, size_t inputSize, ID3D11InputLayout** inputLayout);
	void				SetShaders(Microsoft::WRL::ComPtr<ID3D11PixelShader>& pixelShader, Microsoft::WRL::ComPtr<ID3D11VertexShader>& vertexShader, Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout);
	
	HRESULT CreateConstantBuffer(size_t size, Microsoft::WRL::ComPtr<ID3D11Buffer>& buffer);
	void	UpdateConstantBuffer(SHADER_TYPE shaderType, int slotIndex, Microsoft::WRL::ComPtr<ID3D11Buffer>& destinationData, const void* sourceData);

	void	CreateVertexAndIndexBuffer(Mesh* mesh);
	void	CreateTexture(const void* textureData, Math::Vector2i size, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& resourceView, Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture);

	void	DrawLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Vector3& color = Math::Vector3(1,1,1,1));

	Math::Vector2i	GetWindowSize();
	template<typename T>
	inline UINT GetVectorSize(std::vector<T>& vector)
	{
		return static_cast<UINT>(sizeof(T) * vector.size());
	}
	template<typename T>
	std::pair<D3D11_BUFFER_DESC, D3D11_SUBRESOURCE_DATA>	CreateIndexBuffer(std::vector<T>& vector)
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
		return bufferData;
	}

	template<typename T>
	std::pair<D3D11_BUFFER_DESC, D3D11_SUBRESOURCE_DATA>	CreateVertexBuffer(std::vector<T>& vector)
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
		return bufferData;
	}


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