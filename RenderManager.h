#pragma once
#include "pch.h"
#include "DeviceResources.h"
#include <minwindef.h>
#include <wtypes.h>
#include <DirectXMath.h>
#include <d3d11.h>
#include "Vector3.h"
#include "Matrix.h"

#include "Player.h"
#include <unordered_map>

class Material;

class RenderManager
{
public:


	static RenderManager* GetInstance() {
		static RenderManager instance;
		return &instance;
	}
    Math::Vector2i WindowSize;

	RenderManager();

    void Update();
    void Render();

	std::shared_ptr<DeviceResources> GetDeviceResources() { return m_deviceResources; }
    Math::Matrix GetProjectionMatrix() const;
    Math::Matrix GetViewMatrix() const;
    DirectX::XMFLOAT4X4& GetViewProjectionMatrix() { return m_viewProjectionMatrix; }
    void DrawInstancedMesh(std::shared_ptr<Mesh> mesh, Math::Matrix& matrix);
    HWND GetWindowHandle() const { return m_hWnd; };
    HRESULT CreateVertexAndIndexBuffer(Mesh* mesh);

    Player* GetPlayer() { return m_player; }


    void DrawLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Vector3& color);
     HINSTANCE m_hInstance;

private:

	HRESULT CreateDesktopWindow();

	static LRESULT CALLBACK StaticWindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    void CreateViewAndPerspective();
	void CreateWindowSizeDependentResources();

    void RenderPhysics();

	std::shared_ptr<DeviceResources> m_deviceResources;

	HMENU     m_hMenu;
	HWND      m_hWnd;

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_pInstanceBuffer;
    std::unordered_map<std::shared_ptr<Mesh>, std::vector<DirectX::XMFLOAT4X4>> m_instancedMeshes;

    Player* m_player;

    DirectX::XMMATRIX m_viewMatrix;
    DirectX::XMMATRIX m_projectionMatrix;
    DirectX::XMFLOAT4X4 m_viewProjectionMatrix;

    DirectX::XMFLOAT4 frustumPlanes[6];

};
static std::wstring m_windowClassName;