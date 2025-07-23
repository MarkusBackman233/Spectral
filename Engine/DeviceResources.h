#pragma once
#include "pch.h"
#include <d3d11.h>
#include <wrl.h>
#include <mutex>
#include "Vector2.h"

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

    void CreateResources(HWND hWnd, const Math::Vector2& windowSize);

    void ConfigureBackBuffer(const Math::Vector2& windowSize);
    void ConfigureViewport(const Math::Vector2& windowSize);
    void ReleaseBackBuffer();
    void ReleaseViewport();
    void ResizeSwapchain();

    ID3D11Device* GetDevice() const { return m_pd3dDevice.Get(); };
    LockedContext GetLockedDeviceContext();
    ID3D11RenderTargetView* GetBackBufferTarget() const { return m_pBackbufferRenderTarget.Get(); }
    ID3D11RenderTargetView* GetRenderTarget() const { return m_pRenderTarget.Get(); }

    ID3D11RenderTargetView* GetPostRenderTarget() const { return m_pPostRenderTarget.Get(); }
    ID3D11ShaderResourceView* GetPostSRV() const { return m_pPostSRV.Get(); }

    ID3D11DepthStencilView* GetDepthStencil() const { return m_pDepthStencilView.Get(); }
    ID3D11ShaderResourceView* GetDepthSRV() const { return m_pDepthSRV.Get(); }
    IDXGISwapChain* GetSwapChain() const { return m_pDXGISwapChain.Get(); }

    ID3D11SamplerState* GetDefaultSamplerState() const { return m_defaultSamplerState.Get(); }
    ID3D11SamplerState* GetPointSamplerState() const { return m_pointSamplerState.Get(); }
    ID3D11SamplerState* GetClampSamplerState() const { return m_clampSamplerState.Get(); }
    ID3D11BlendState*   GetDefaultBlendState() const { return m_defaultBlendState.Get(); }
    ID3D11BlendState*   GetTransparentBlendState() const { return m_transarentBlendState.Get(); }

    ID3D11ShaderResourceView* RenderTargetSRV() const { return m_pRenderTargetSRV.Get(); }

    ID3D11RasterizerState* GetBackfaceCullingRasterizer() const { return m_backfaceCullingRasterizer.Get(); }
    ID3D11RasterizerState* GetNoCullingRasterizer() const { return m_noCullingRasterizer.Get(); }


    void Present();

private:
    void CreateWindowResources(HWND hWnd, const Math::Vector2& windowSize);
    void CreateDeviceResources();

    Microsoft::WRL::ComPtr<ID3D11Device>        m_pd3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pd3dDeviceContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain>      m_pDXGISwapChain;


    Microsoft::WRL::ComPtr<ID3D11Texture2D>        m_pBackBuffer;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pBackbufferRenderTarget;

    Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_pRenderTexture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   m_pRenderTarget;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pRenderTargetSRV;


    Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_pPostProcessingRenderTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pPostSRV;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   m_pPostRenderTarget;


    Microsoft::WRL::ComPtr<ID3D11Texture2D>         m_pDepthStencil;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_pDepthStencilView;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>   m_pDepthSRV;

    D3D_FEATURE_LEVEL       m_featureLevel;
    D3D11_TEXTURE2D_DESC    m_bbDesc;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_defaultSamplerState;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pointSamplerState;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_clampSamplerState;
    Microsoft::WRL::ComPtr<ID3D11BlendState>   m_defaultBlendState;
    Microsoft::WRL::ComPtr<ID3D11BlendState>   m_transarentBlendState;

    Microsoft::WRL::ComPtr<ID3D11RasterizerState>   m_backfaceCullingRasterizer;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState>   m_noCullingRasterizer;
};