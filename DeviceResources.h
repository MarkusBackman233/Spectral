#pragma once
#include "pch.h"
#include <d3d11.h>
#include <wrl.h>
#include <mutex>

class LockedContext
{
public:
    LockedContext(ID3D11DeviceContext* context, std::mutex* mutex) : m_context(context), m_mutex(mutex) {};
    ~LockedContext() { m_mutex->unlock(); }
    ID3D11DeviceContext* GetContext() { return m_context; }

private:
    ID3D11DeviceContext* m_context;
    std::mutex* m_mutex;
};

class DeviceResources
{
public:
    DeviceResources();


    HRESULT CreateDeviceResources();
    HRESULT CreateWindowResources(HWND hWnd);

    HRESULT ConfigureBackBuffer(HWND hWnd);
    HRESULT ReleaseBackBuffer();

    ID3D11Device* GetDevice() { return m_pd3dDevice.Get(); };
    LockedContext GetLockedDeviceContext();
    ID3D11RenderTargetView* GetRenderTarget() { return m_pRenderTarget.Get(); }
    ID3D11DepthStencilView* GetDepthStencil() { return m_pDepthStencilView.Get(); }
    IDXGISwapChain* GetSwapChain() { return m_pDXGISwapChain.Get(); }

    ID3D11SamplerState* GetDefaultSamplerState() { return m_defaultSamplerState.Get(); }

    void Present();

private:

    //-----------------------------------------------------------------------------
    // Direct3D device
    //-----------------------------------------------------------------------------
    Microsoft::WRL::ComPtr<ID3D11Device>        m_pd3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pd3dDeviceContext; // immediate context
    Microsoft::WRL::ComPtr<IDXGISwapChain>      m_pDXGISwapChain;


    //-----------------------------------------------------------------------------
    // DXGI swap chain device resources
    //-----------------------------------------------------------------------------
    Microsoft::WRL::ComPtr<ID3D11Texture2D>        m_pBackBuffer;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pRenderTarget;


    //-----------------------------------------------------------------------------
    // Direct3D device resources for the depth stencil
    //-----------------------------------------------------------------------------
    Microsoft::WRL::ComPtr<ID3D11Texture2D>         m_pDepthStencil;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_pDepthStencilView;
    //-----------------------------------------------------------------------------
    // Direct3D device metadata and device resource metadata
    //-----------------------------------------------------------------------------
    D3D_FEATURE_LEVEL       m_featureLevel;
    D3D11_TEXTURE2D_DESC    m_bbDesc;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_defaultSamplerState;

    //std::mutex m_contextMutex;
    
};