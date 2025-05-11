#include "RenderManager.h"
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
    m_shadowManager.CreateResources(device);
    m_lineRenderer.CreateResources(device);
    m_FXAA.CreateResources(device);
    m_guizmoRenderer.CreateResources(device);
    ID3D11SamplerState* samplers[5] = {
        m_deviceResources.GetDefaultSamplerState(),
        m_shadowManager.GetShadowCompareSamplerState(),
        m_skyboxManager.GetCubeSamplerState(),
        m_deviceResources.GetClampSamplerState(),
        m_deviceResources.GetPointSamplerState()
    };
    context->PSSetSamplers(0, 5, samplers);
    m_camera = std::make_unique<PerspectiveCamera>(75.0f, 0.1f, 1000.0f, windowSize);
}

void RenderManager::OnWindowResize()
{
    m_deviceResources.ReleaseBackBuffer();
    m_deferredPipeline.ReleaseResources();
    m_SSAO.ReleaseResources();
    m_deviceResources.ResizeSwapchain();

    Math::Vector2 windowSize = Render::GetWindowSizef();
    ID3D11Device* device = m_deviceResources.GetDevice();

    m_deferredPipeline.CreateResources(device, windowSize);
    m_SSAO.CreateResources(device, windowSize);
    m_deviceResources.ConfigureBackBuffer(windowSize);
    m_camera->UpdateAspectRatio(windowSize);
}

void RenderManager::Render()
{
    LockedContext lockedContext = m_deviceResources.GetLockedDeviceContext();
    ID3D11DeviceContext* context = lockedContext.GetContext();

    m_skyboxManager.CreateCubeMap(context, m_deviceResources.GetDevice());
    m_camera->CreateViewAndPerspective();
    m_instanceManager.Map(context, m_deviceResources.GetDevice());
    m_shadowManager.DrawShadowDepth(context, m_instanceManager);
    m_deferredPipeline.RenderGBuffer(context, m_deviceResources, m_instanceManager, m_shadowManager);
    m_SSAO.Process(context, m_deviceResources, m_deferredPipeline);
    m_skyboxManager.RenderSkybox(context, m_deviceResources.GetRenderTarget());
    m_pbrRender.Process(context, m_deviceResources, m_deferredPipeline, m_skyboxManager, m_shadowManager, m_SSAO);
    m_FXAA.Process(context, m_deviceResources);
    m_lineRenderer.Render(context, m_deviceResources);


    m_guizmoRenderer.Render(context, m_deviceResources);

    //ID3D11RenderTargetView* nulltargets[] = { nullptr };
    //context->OMSetRenderTargets(1, nulltargets, nullptr);
}

void RenderManager::Present()
{
    Render::CopyRenderTarget(m_deviceResources.GetLockedDeviceContext().GetContext(), m_deviceResources.GetBackBufferTarget(), m_deviceResources.GetRenderTarget());
    m_deviceResources.Present();
}