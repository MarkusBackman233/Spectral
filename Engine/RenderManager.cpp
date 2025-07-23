#include "RenderManager.h"
#include "DefaultMaterial.h"
#include "TerrainMaterial.h"

RenderManager::RenderManager()
{
#if defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    m_windowsManager.CreateDesktopWindow();
    HWND windowHandle = m_windowsManager.GetWindowHandle();
    Math::Vector2 windowSize = m_windowsManager.GetWindowSize();

    m_deviceResources.CreateResources(windowHandle, windowSize);
    ID3D11Device* device = m_deviceResources.GetDevice();
    LockedContext lockedContext = m_deviceResources.GetLockedDeviceContext();
    ID3D11DeviceContext* context = lockedContext.GetContext();

    m_guiManager.CreateResources(device, windowHandle, context);
    m_deferredPipeline.CreateResources(device, windowSize);
    m_SSAO.CreateResources(device, windowSize);
    m_pbrRender.CreateResources(device);
    m_skyboxManager.CreateResources(context, device);
    m_cloudGenerator.CreateResources(device);
    m_shadowManager.CreateResources(device);
    m_lineRenderer.CreateResources(device);
    m_FXAA.CreateResources(device);
    m_guizmoRenderer.CreateResources(device);
    m_grassRenderer.CreateResources(device);

    DefaultMaterial::CreateResources(device);
    TerrainMaterial::CreateResources(device);


    ID3D11SamplerState* samplers[5] = {
        m_deviceResources.GetDefaultSamplerState(),
        m_shadowManager.GetShadowCompareSamplerState(),
        m_skyboxManager.GetCubeSamplerState(),
        m_deviceResources.GetClampSamplerState(),
        m_deviceResources.GetPointSamplerState()
    };
    context->PSSetSamplers(0, 5, samplers);
    m_camera = std::make_unique<PerspectiveCamera>(75.0f, 0.1f, 1000.0f, windowSize);
    OnViewportResize(Math::Vector2(0.0f, 0.0f), windowSize);
}

void RenderManager::OnWindowResize(Math::Vector2 newSize)
{
    m_deviceResources.ReleaseBackBuffer();
    m_deviceResources.ResizeSwapchain();
    m_deviceResources.ConfigureBackBuffer(newSize);
}

void RenderManager::OnViewportResize(Math::Vector2 topLeft, Math::Vector2 newSize)
{
    m_deviceResources.ReleaseViewport();
    m_deferredPipeline.ReleaseResources();
    m_SSAO.ReleaseResources();
    

    ID3D11Device* device = m_deviceResources.GetDevice();
    m_deviceResources.ConfigureViewport(newSize);
    m_deferredPipeline.CreateResources(device, newSize);
    m_SSAO.CreateResources(device, newSize);
    m_camera->UpdateAspectRatio(newSize);

    m_currentViewportSize = newSize;
    m_currentViewportPos = topLeft;


}

void RenderManager::Render()
{
    LockedContext lockedContext = m_deviceResources.GetLockedDeviceContext();
    ID3D11DeviceContext* context = lockedContext.GetContext();

    m_skyboxManager.RenderCubeMap(context, m_deviceResources.GetDevice());
    m_camera->CreateViewAndPerspective();
    m_instanceManager.Map(context, m_deviceResources.GetDevice());
    m_shadowManager.DrawShadowDepth(context, m_instanceManager);

    D3D11_VIEWPORT viewport{};
    viewport.Width = m_currentViewportSize.x;
    viewport.Height = m_currentViewportSize.y;
    viewport.MaxDepth = 1.0f;
    context->RSSetViewports(1, &viewport);
    context->ClearRenderTargetView(m_deviceResources.GetBackBufferTarget(), Math::Vector4(0, 0, 0, 1).Data());
    m_deferredPipeline.RenderGBuffer(context, m_deviceResources, m_instanceManager, m_shadowManager);
    m_grassRenderer.Render(context, m_deviceResources);
    m_SSAO.Process(context, m_deviceResources, m_deferredPipeline);
    m_skyboxManager.RenderSkybox(context, m_deviceResources.GetRenderTarget());
    m_cloudGenerator.Render(context, m_deviceResources, m_skyboxManager);
    m_pbrRender.Process(context, m_deviceResources, m_deferredPipeline, m_skyboxManager, m_shadowManager, m_SSAO);
    m_FXAA.Process(context, m_deviceResources);
    m_lineRenderer.Render(context, m_deviceResources);
    m_guizmoRenderer.Render(context, m_deviceResources);

#ifdef EDITOR
    ID3D11RenderTargetView* renderTarget[] = { m_deviceResources.GetBackBufferTarget() };
    context->OMSetRenderTargets(1, renderTarget, nullptr);

#endif
}

void RenderManager::Present()
{
    m_deviceResources.Present();
}