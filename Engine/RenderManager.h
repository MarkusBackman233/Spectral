#pragma once
#include "pch.h"
#include "DeviceResources.h"
#include "PerspectiveCamera.h"
#include "FXAA.h"
#include "SSAO.h"
#include "DeferredPipeline.h"
#include "PbrRender.h"
#include "SkyboxManager.h"
#include "ShadowManager.h"
#include "LineRenderer.h"
#include "GuizmoRenderer.h"
#include "WindowsManager.h"
#include "InstanceManager.h"
#include "GuiManager.h"

class RenderManager
{
public:
	static RenderManager* GetInstance() {
		static RenderManager instance;
		return &instance;
	}
	RenderManager();
	void OnWindowResize();
    void Render();
    void Present();

	InstanceManager*	GetInstanceManager()	{ return &m_instanceManager; }
	GuizmoRenderer*		GetGuizmoRenderer()		{ return &m_guizmoRenderer; }
	LineRenderer*		GetLineRenderer()		{ return &m_lineRenderer; }
	WindowsManager*		GetWindowsManager()		{ return &m_windowsManager; }
	DeviceResources*	GetDeviceResources()	{ return &m_deviceResources; }
	GuiManager*			GetGuiManager()			{ return &m_guiManager; }
	PbrRender*			GetPbrRenderer()		{ return &m_pbrRender; }
	PerspectiveCamera*	GetCamera()				{ return m_camera.get(); }

private:
	WindowsManager m_windowsManager;
	DeviceResources m_deviceResources;
	DeferredPipeline m_deferredPipeline;
	InstanceManager m_instanceManager;
	PbrRender m_pbrRender;
	SkyboxManager m_skyboxManager;
	ShadowManager m_shadowManager;
	LineRenderer m_lineRenderer;
	GuizmoRenderer m_guizmoRenderer;
	GuiManager m_guiManager;
    FXAA m_FXAA;
    SSAO m_SSAO;
	std::unique_ptr<PerspectiveCamera> m_camera;
};
