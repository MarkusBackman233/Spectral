#pragma once
#include <wrl/client.h>

#include "Vector3.h"
#include "Vector2.h"

struct ID3D11Buffer;
struct ID3D11PixelShader;
struct ID3D11VertexShader;
struct ID3D11InputLayout;
struct ID3D11ShaderResourceView;
struct ID3D11DeviceContext;
struct ID3D11Device;

class DeviceResources;

class PostProcessing
{
public:
	PostProcessing();


protected:
	void CreatePostProcessingResources(ID3D11Device* device);
	void SetVertexBuffer(ID3D11DeviceContext* context);

	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
private:
	struct Vertex 
	{
		Math::Vector3 position;
		Math::Vector2 texCoord;
	};

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderRV;
};

