#include "pch.h"

#include "DeviceResources.h"
#include <dxgi1_3.h>
#include <mutex>
#include "iRender.h"
#include "Vector2.h"

DeviceResources::DeviceResources()
{
}

void DeviceResources::CreateResources(HWND hWnd, const Math::Vector2& windowSize)
{
    CreateDeviceResources();
    CreateWindowResources(hWnd, windowSize);
}

void DeviceResources::CreateDeviceResources()
{
    D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_1 };

    UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(DEBUG) || defined(_DEBUG)
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // Create the Direct3D 11 API device object and a corresponding context.
    Microsoft::WRL::ComPtr<ID3D11Device>        device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    ThrowIfFailed(
        D3D11CreateDevice(
            nullptr,                    // Specify nullptr to use the default adapter.
            D3D_DRIVER_TYPE_HARDWARE,   // Create a device using the hardware graphics driver.
            0,                          // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
            deviceFlags,                // Set debug and Direct2D compatibility flags.
            levels,                     // List of feature levels this app can support.
            ARRAYSIZE(levels),          // Size of the list above.
            D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
            &device,                    // Returns the Direct3D device created.
            &m_featureLevel,            // Returns feature level of device created.
            &context                    // Returns the device immediate context.
        )
    );
    // Store pointers to the Direct3D 11.1 API device and immediate context.
    device.As(&m_pd3dDevice);
    context.As(&m_pd3dDeviceContext);
    {
        Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // Linear filtering
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // Wrap addressing mode
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP; // Wrap addressing mode
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; // Wrap addressing mode
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.MaxAnisotropy = 16;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        
        
        ThrowIfFailed(device->CreateSamplerState(&samplerDesc, &samplerState));
        samplerState.As(&m_defaultSamplerState);    
    }    
    {
        Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; // Linear filtering
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // Wrap addressing mode
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP; // Wrap addressing mode
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; // Wrap addressing mode
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.MaxAnisotropy = 16;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        
        
        ThrowIfFailed(device->CreateSamplerState(&samplerDesc, &samplerState));
        samplerState.As(&m_pointSamplerState);    
    }
    {
        Microsoft::WRL::ComPtr<ID3D11SamplerState> clampSamplerState;
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // Linear filtering
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP; 
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        
        ThrowIfFailed(device->CreateSamplerState(&samplerDesc, &clampSamplerState));
        clampSamplerState.As(&m_clampSamplerState);
    }

    {
        D3D11_RASTERIZER_DESC rasterizerDesc;
        ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
        rasterizerDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerDesc.CullMode = D3D11_CULL_BACK;
        device->CreateRasterizerState(&rasterizerDesc, m_backfaceCullingRasterizer.GetAddressOf());    
        rasterizerDesc.CullMode = D3D11_CULL_NONE;
        device->CreateRasterizerState(&rasterizerDesc, m_noCullingRasterizer.GetAddressOf());
        context->RSSetState(m_backfaceCullingRasterizer.Get());
    }

    {
        D3D11_BLEND_DESC blendDesc;
        ZeroMemory(&blendDesc, sizeof(blendDesc));
        blendDesc.AlphaToCoverageEnable = false;
        blendDesc.IndependentBlendEnable = false;

        blendDesc.RenderTarget[0].BlendEnable = false;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        ThrowIfFailed(device->CreateBlendState(&blendDesc, m_defaultBlendState.GetAddressOf()));
        blendDesc.RenderTarget[0].BlendEnable = true;
        ThrowIfFailed(device->CreateBlendState(&blendDesc, m_transarentBlendState.GetAddressOf()));
    }
}

void DeviceResources::CreateWindowResources(HWND hWnd, const Math::Vector2& windowSize)
{
    DXGI_SWAP_CHAIN_DESC desc;
    ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));
    desc.Windowed = TRUE;
    desc.BufferCount = 2;
    desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.OutputWindow = hWnd;
    

    Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice;
    m_pd3dDevice.As(&dxgiDevice);

    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    ThrowIfFailed(dxgiDevice->GetAdapter(&adapter));

    Microsoft::WRL::ComPtr<IDXGIFactory> factory;
    adapter->GetParent(IID_PPV_ARGS(&factory));

    ThrowIfFailed(factory->CreateSwapChain(m_pd3dDevice.Get(),&desc,&m_pDXGISwapChain));

    ConfigureBackBuffer(windowSize);
}

void DeviceResources::ConfigureBackBuffer(const Math::Vector2& windowSize)
{
    ThrowIfFailed(m_pDXGISwapChain->GetBuffer(0,__uuidof(ID3D11Texture2D),(void**)&m_pBackBuffer));
    ThrowIfFailed(m_pd3dDevice->CreateRenderTargetView(m_pBackBuffer.Get(),nullptr, m_pBackbufferRenderTarget.GetAddressOf()));

    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(textureDesc));
    textureDesc.Width = static_cast<UINT>(windowSize.x);
    textureDesc.Height = static_cast<UINT>(windowSize.y);
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = 0;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
    ThrowIfFailed(m_pd3dDevice->CreateTexture2D(&textureDesc, NULL, m_pPostProcessingRenderTexture.GetAddressOf()));

    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    ThrowIfFailed(m_pd3dDevice->CreateTexture2D(&textureDesc, NULL, m_pRenderTexture.GetAddressOf()));
    ThrowIfFailed(m_pd3dDevice->CreateRenderTargetView(m_pPostProcessingRenderTexture.Get(), nullptr, m_pPostRenderTarget.GetAddressOf()));
    ThrowIfFailed(m_pd3dDevice->CreateRenderTargetView(m_pRenderTexture.Get(), nullptr, m_pRenderTarget.GetAddressOf()));
    
    
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
    shaderResourceViewDesc.Format = textureDesc.Format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;
    ThrowIfFailed(m_pd3dDevice->CreateShaderResourceView(m_pRenderTexture.Get(), &shaderResourceViewDesc, m_pRenderTargetSRV.GetAddressOf()));

    m_pBackBuffer.Reset();
    m_pRenderTexture.Reset();
    m_pPostProcessingRenderTexture.Reset();
    

    D3D11_TEXTURE2D_DESC depthStencilDesc{};
    depthStencilDesc.Width = static_cast<UINT>(windowSize.x);
    depthStencilDesc.Height = static_cast<UINT>(windowSize.y);
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; // Support for both depth and shader resource
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    D3D11_SHADER_RESOURCE_VIEW_DESC depthSRVDesc{};
    depthSRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    depthSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    depthSRVDesc.Texture2D.MipLevels = 1;
    depthSRVDesc.Texture2D.MostDetailedMip = 0;
    ThrowIfFailed(m_pd3dDevice->CreateTexture2D(&depthStencilDesc,nullptr, &m_pDepthStencil));
    ThrowIfFailed(m_pd3dDevice->CreateDepthStencilView(m_pDepthStencil.Get(),&depthStencilViewDesc, &m_pDepthStencilView));
    ThrowIfFailed(m_pd3dDevice->CreateShaderResourceView(m_pDepthStencil.Get(), &depthSRVDesc, &m_pDepthSRV));

    D3D11_VIEWPORT viewport{};
    viewport.Width = static_cast<FLOAT>(windowSize.x);
    viewport.Height = static_cast<FLOAT>(windowSize.y);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    m_pd3dDeviceContext->RSSetViewports(1, &viewport);
}

void DeviceResources::ReleaseBackBuffer()
{
    m_pBackBuffer.Reset();
    m_pRenderTarget.Reset();
    m_pBackbufferRenderTarget.Reset();
    m_pPostRenderTarget.Reset();
    m_pRenderTexture.Reset();
    m_pPostProcessingRenderTexture.Reset();
    m_pRenderTargetSRV.Reset();
    m_pDepthStencilView.Reset();
    m_pDepthStencil.Reset();
    m_pd3dDeviceContext->Flush();
}

void DeviceResources::ResizeSwapchain()
{
    ThrowIfFailed(m_pDXGISwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0));
}

LockedContext DeviceResources::GetLockedDeviceContext()
{
    static std::mutex contextMutex;
    contextMutex.lock();
    return LockedContext(m_pd3dDeviceContext.Get(), &contextMutex);
}

void DeviceResources::Present()
{
    m_pDXGISwapChain->Present(0, 0);
}
