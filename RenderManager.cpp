#include <string>
#include <memory>
#include <ppltasks.h>

#include "RenderManager.h"
#include <math.h>

#include <string>
#include <memory>
#include <comdef.h>
#include "Mesh.h"
#include "ShadowManager.h"

#include <iostream>
#include <assert.h>
#include <fstream>
#include "src/IMGUI/imgui_impl_win32.h"
#include "src/IMGUI/imgui_impl_dx11.h"
#include <DirectXCollision.h>

#include "Editor.h"
#include "ObjectManager.h"
#include "InputManager.h"
#include <d3d11.h>
#include "LineMaterial.h"
#include "MaterialManager.h"
#include "iRender.h"
#include "Spectral.h"
#include "ProfilerManager.h"
#include "SkyboxManager.h"
#include <tchar.h>

RenderManager::RenderManager()
{
    m_deviceResources = std::make_shared<DeviceResources>();
    m_windowClassName = L"SpectralWindowClass";
    m_hInstance = NULL;
    HRESULT hr = S_OK;
    m_player = new Player();
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    hr = CreateDesktopWindow();

    if (SUCCEEDED(hr))
    {
        m_deviceResources->CreateDeviceResources();
        m_deviceResources->CreateWindowResources(GetWindowHandle());
        CreateWindowSizeDependentResources();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(GetWindowHandle());
        ImGui_ImplDX11_Init(GetDeviceResources()->GetDevice(), GetDeviceResources()->GetLockedDeviceContext().GetContext());
    }
}


Math::Matrix RenderManager::GetProjectionMatrix() const
{
    
    return *(Math::Matrix*)((void*)&m_projectionMatrix);
}

Math::Matrix RenderManager::GetViewMatrix() const
{
    return *(Math::Matrix*)((void*)&m_viewMatrix);
}

void RenderManager::DrawInstancedMesh(std::shared_ptr<Mesh> mesh, Math::Matrix& matrix)
{
    DirectX::XMFLOAT4X4 instanceMatrix;
    DirectX::XMStoreFloat4x4(&instanceMatrix, DirectX::XMMatrixTranspose(*(DirectX::XMMATRIX*)((void*)&matrix)));
    m_instancedMeshes[mesh].emplace_back(instanceMatrix);
}

HRESULT RenderManager::CreateVertexAndIndexBuffer(Mesh* mesh)
{
    HRESULT hr = S_OK;
    ID3D11Device* device = m_deviceResources->GetDevice();
    
    auto [vertexBufferDesc, vertexBufferData] =  Render::CreateVertexBuffer(mesh->vertexes);
    ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc,&vertexBufferData,&mesh->m_pVertexBuffer));
    
    auto [indexBufferDesc, indexBufferData] = Render::CreateIndexBuffer(mesh->indices32);
    ThrowIfFailed(device->CreateBuffer(&indexBufferDesc,&indexBufferData,&mesh->m_pIndexBuffer));

    return hr;
}

HRESULT RenderManager::CreateDesktopWindow()
{
    // Window resources are dealt with here.

    if (m_hInstance == NULL)
        m_hInstance = (HINSTANCE)GetModuleHandle(NULL);

    HICON hIcon = NULL;
    WCHAR szExePath[MAX_PATH];
    GetModuleFileName(NULL, szExePath, MAX_PATH);

    // If the icon is NULL, then use the first one found in the exe
    if (hIcon == NULL)
        hIcon = ExtractIcon(m_hInstance, szExePath, 0);

    // Register the windows class
    WNDCLASS wndClass;
    wndClass.style = CS_DBLCLKS;
    wndClass.lpfnWndProc = RenderManager::StaticWindowProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = m_hInstance;
    wndClass.hIcon = hIcon;
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = m_windowClassName.c_str();

    if (!RegisterClass(&wndClass))
    {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_CLASS_ALREADY_EXISTS)
            return HRESULT_FROM_WIN32(dwError);
    }

    int x = CW_USEDEFAULT;
    int y = CW_USEDEFAULT;

    // No menu in this example.
    m_hMenu = NULL;

    // This example uses a non-resizable 640 by 480 viewport for simplicity.
    int nDefaultWidth = 1280;
    int nDefaultHeight = 720;
    RECT rc{};
    SetRect(&rc, 0, 0, nDefaultWidth, nDefaultHeight);
    AdjustWindowRect(&rc,WS_OVERLAPPEDWINDOW,(m_hMenu != NULL) ? true : false);

    // Create the window for our viewport.
    m_hWnd = CreateWindow(
        m_windowClassName.c_str(),
        L"Spectral",
        WS_OVERLAPPEDWINDOW,
        x, y,
        (rc.right - rc.left), (rc.bottom - rc.top),
        0,
        m_hMenu,
        m_hInstance,
        0
    );
    DragAcceptFiles(m_hWnd, TRUE);
    if (m_hWnd == NULL)
    {
        DWORD dwError = GetLastError();
        return HRESULT_FROM_WIN32(dwError);
    }

    return S_OK;
}



extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK RenderManager::StaticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;
    switch (uMsg)
    {
    case WM_CLOSE:
    {
        HMENU hMenu;
        hMenu = GetMenu(hWnd);
        if (hMenu != NULL)
        {
            DestroyMenu(hMenu);
        }
        DestroyWindow(hWnd);
        UnregisterClass(m_windowClassName.c_str(), RenderManager::GetInstance()->m_hInstance);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;


    case WM_SIZE:
    {
        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;
        if (width == 0 || height == 0)
        {
            return 0;
        }
        RenderManager::GetInstance()->WindowSize.x = width;
        RenderManager::GetInstance()->WindowSize.y = height;
        RenderManager::GetInstance()->GetDeviceResources()->ReleaseBackBuffer();
        auto hr = RenderManager::GetInstance()->GetDeviceResources()->GetSwapChain()->ResizeBuffers(
            0,                   // Number of buffers. Set this asto 0 to preserve the existing setting.
            0, 0,                // Width and height of the swap chain.aS Set to 0 to match the screen resolution.
            DXGI_FORMAT_UNKNOWN, // This tells DXGI to retain the current back buffer format.
            0
        );
        RenderManager::GetInstance()->GetDeviceResources()->ConfigureBackBuffer(RenderManager::GetInstance()->GetWindowHandle());
        break;
    }    
    case WM_DROPFILES:
    {
        HDROP hDrop = (HDROP)wParam;
        UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

        for (UINT i = 0; i < fileCount; ++i) {
            TCHAR filePath[MAX_PATH];
            DragQueryFile(hDrop, i, filePath, MAX_PATH);
            int bufferSize = WideCharToMultiByte(CP_UTF8, 0, filePath, -1, NULL, 0, NULL, NULL);
            std::string filename(bufferSize, '\0');
            WideCharToMultiByte(CP_UTF8, 0, filePath, -1, filename.data(), bufferSize, NULL, NULL);
            Editor::GetInstance()->HandleDropFile(filename);
        }

        DragFinish(hDrop);
        break;
    }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


void RenderManager::CreateViewAndPerspective()
{
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.f);
    DirectX::XMVECTOR eye = DirectX::XMVectorSet(m_player->GetPosition().x, m_player->GetPosition().y, m_player->GetPosition().z, 0.f);
    DirectX::XMVECTOR at = DirectX::XMVectorSet(m_player->GetCameraMatrix().Front().x, m_player->GetCameraMatrix().Front().y, m_player->GetCameraMatrix().Front().z, 0.f);
    at = DirectX::XMVectorAdd(at, eye);
    m_viewMatrix = DirectX::XMMatrixLookAtRH(eye,at,up);

    float fieldOfView = DirectX::XMConvertToRadians(75.0f);
    RECT rc;
    GetClientRect(GetWindowHandle(), &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;
    m_projectionMatrix = DirectX::XMMatrixPerspectiveFovRH(fieldOfView, (float)width / height, 0.1f, 3000.0f);
    DirectX::XMStoreFloat4x4(&m_viewProjectionMatrix, DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(m_viewMatrix, m_projectionMatrix)));
}

void RenderManager::CreateWindowSizeDependentResources()
{
    CreateViewAndPerspective();
}
void RenderManager::RenderPhysics()
{
    const PxRenderBuffer& rb = PhysXManager::GetInstance()->GetScene()->getRenderBuffer();

    for (PxU32 i = 0; i < rb.getNbLines(); i++)
    {
        const PxDebugLine& line = rb.getLines()[i];

        Math::Vector3 p0(line.pos0);
        Math::Vector3 p1(line.pos1);

        DrawLine(p0, p1, Math::Vector3(1, 1, 1, 1));
    }
}
void RenderManager::Update()
{
    m_player->Update(Spectral::GetInstance()->GetDeltaTime());
    CreateViewAndPerspective();
}

void RenderManager::Render()
{
    auto timer = ProfilerManager::GetInstance()->GetProfileObject(__func__);
    timer.StartTimer();
    ShadowManager::GetInstance()->DrawShadowDepth(m_instancedMeshes);
    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(blendDesc));

    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;

    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    // Create the blend state
    ID3D11BlendState* pBlendState;
    HRESULT hr = Render::GetDevice()->CreateBlendState(&blendDesc, &pBlendState);

    {
        auto lockedContext = m_deviceResources->GetLockedDeviceContext();
        ID3D11DeviceContext*    context = lockedContext.GetContext();
        ID3D11RenderTargetView* renderTarget = m_deviceResources->GetRenderTarget();
        ID3D11DepthStencilView* depthStencil = m_deviceResources->GetDepthStencil();
        float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        UINT sampleMask = 0xffffffff;
        context->OMSetBlendState(pBlendState, blendFactor, sampleMask);

        D3D11_VIEWPORT viewport;
        viewport.Width = static_cast<float>(Render::GetWindowSize().x);
        viewport.Height = static_cast<float>(Render::GetWindowSize().y);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;

        context->RSSetViewports(1, &viewport);

        ID3D11RenderTargetView* nullRenderTarget = nullptr; // Null render target
        context->OMSetRenderTargets(1, &renderTarget, depthStencil );
        const float dark[] = { 0.098f, 0.098f, 0.098f, 1.000f };
        context->ClearRenderTargetView(renderTarget, dark);
        context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->PSSetShaderResources(5, 1, ShadowManager::GetInstance()->GetShadowTexture().GetAddressOf()); // shadow map
    }
    MaterialManager::GetInstance()->GetDefaultMaterial()->PrepareMaterialGlobals();
    //{
    //    MaterialManager::GetInstance()->GetDefaultMaterial()->PreparePerMaterial();
    //    SkyboxManager::GetInstance()->RenderSkybox();
    //
    //    auto lockedContext = Render::GetContext();
    //    ID3D11DeviceContext* context = lockedContext.GetContext();
    //    context->DrawIndexed(36, 0, 0);
    //}

    UINT stride = sizeof(Mesh::VertexStruct);
    UINT offset = 0;

    for (auto& [mesh, matrixes] : m_instancedMeshes)
    {
        mesh->GetMaterial()->PreparePerMaterial();
        auto lockedContext = m_deviceResources->GetLockedDeviceContext();
        ID3D11DeviceContext* context = lockedContext.GetContext();

        context->IASetVertexBuffers(0, 1, mesh->m_pVertexBuffer.GetAddressOf(), &stride, &offset);
        context->IASetIndexBuffer(mesh->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

        auto [instanceBufferDesc, instanceBufferData] = Render::CreateVertexBuffer(matrixes);
        m_deviceResources->GetDevice()->CreateBuffer(&instanceBufferDesc, &instanceBufferData, &m_pInstanceBuffer);
        UINT strideInstance = sizeof(DirectX::XMFLOAT4X4);
        context->IASetVertexBuffers(1, 1, m_pInstanceBuffer.GetAddressOf(), &strideInstance, &offset);
        context->DrawIndexedInstanced(static_cast<UINT>(mesh->indices32.size()), static_cast<UINT>(matrixes.size()), 0, 0, 0);
    }
    
    m_instancedMeshes.clear();
    RenderPhysics();
    pBlendState->Release();
}

void RenderManager::SetWindowSize(const Math::Vector2i& size)
{
    RECT desktop;
    // Get a handle to the desktop window
    const HWND hDesktop = GetDesktopWindow();
    // Get the size of screen to the variable desktop
    GetWindowRect(hDesktop, &desktop);
    ::SetWindowLong(GetWindowHandle(), GWL_STYLE, GetWindowLong(GetWindowHandle(), GWL_STYLE) & ~WS_SIZEBOX);

    MoveWindow(GetWindowHandle(),
        desktop.right / 2 - size.x / 2,
        desktop.bottom / 2 - size.y / 2, size.x,
        size.y, TRUE);
    
}

void RenderManager::SetWindowIcon(const std::string& iconName)
{
    HANDLE hIcon = LoadImageA(0, iconName.c_str(), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
    SendMessage(GetWindowHandle(), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    SendMessage(GetWindowHandle(), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
}

void RenderManager::SetWindowTitle(const std::string& windowTitle)
{
    SetWindowTextA(GetWindowHandle(), windowTitle.c_str());
}

void RenderManager::DrawLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Vector3& color)
{
    MaterialManager::GetInstance()->GetLineMaterial()->PreparePerMaterial();
    std::static_pointer_cast<LineMaterial>(MaterialManager::GetInstance()->GetLineMaterial())->SetColor(color);
    auto lockedContext = m_deviceResources->GetLockedDeviceContext();
    ID3D11DeviceContext* context = lockedContext.GetContext();

    ID3D11RenderTargetView* renderTarget = m_deviceResources->GetRenderTarget();
    ID3D11DepthStencilView* depthStencil = m_deviceResources->GetDepthStencil();

    context->OMSetRenderTargets(1, &renderTarget, depthStencil);
    std::vector<float> vertexes {start.x, start.y, start.z,end.x, end.y, end.z};


    auto [vertexBufferDesc, vertexBufferData] = Render::CreateVertexBuffer(vertexes);

    ID3D11Buffer* vertexBuffer;
    m_deviceResources->GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertexBuffer);

    UINT stride = 3 * sizeof(float);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    context->Draw(2, 0);
    vertexBuffer->Release();
}
