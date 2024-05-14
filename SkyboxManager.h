#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

class SkyboxManager
{
public:
	static SkyboxManager* GetInstance() {
		static SkyboxManager instance;
		return &instance;
	}

	SkyboxManager();

	void RenderSkybox();
private:

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer;
};

