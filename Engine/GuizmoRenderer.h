#pragma once
#include <d3d11.h>
#include "wrl.h"
#include "Material.h"
#include "Matrix.h"
class Texture;
class DeviceResources;

class GuizmoRenderer
{
public:
	GuizmoRenderer();

	void CreateResources(ID3D11Device* device);

	void Render(ID3D11DeviceContext* context, const DeviceResources& deviceResources);

	void AddGuizmo(std::shared_ptr<Texture> texture, const Math::Vector3& position, const Math::Vector4& color);

private:
	Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_pInputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pVertexConstantBufferData;
	Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pPixelConstantBufferData;

	struct Vertex
	{
		Math::Vector3 Position;
		Math::Vector2 TexCoord;
	};

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer;

	std::unordered_map<std::shared_ptr<Texture>, std::vector<std::pair<Math::Vector3, Math::Vector4>>> m_guizmosToDraw;

};

