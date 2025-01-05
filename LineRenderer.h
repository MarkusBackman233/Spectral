#pragma once
#include <d3d11.h>
#include "wrl.h"
#include "Vector3.h"
#include "Vector4.h"
#include <vector>
#include "ConstantBuffer.h"
#include "Matrix.h"
class DeviceResources;

class LineRenderer
{
public:
	void CreateResources(ID3D11Device* device);
	void Render(ID3D11DeviceContext* context, const DeviceResources& deviceResources);
	void AddLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Vector4& color);

private:
	struct Vertex
	{
		Vertex(const Math::Vector3& position, const Math::Vector4& color)
		{
			memcpy(&Position, &position, sizeof(Position));
			memcpy(&Color, &color, sizeof(Color));
		}
		Math::Vector3 Position;
		Math::Vector4 Color;
	};

	Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_pInputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pVertexConstantBufferData;
	Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pPixelConstantBufferData;

	std::vector<Vertex> m_linesToDraw;
};

