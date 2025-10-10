#pragma once
#include "pch.h"
#include "DeviceResources.h"
#include "PerspectiveCamera.h"
#include "FXAA.h"
#include "DeferredPipeline.h"
#include "PbrRender.h"
#include "SkyboxManager.h"
#include "ShadowManager.h"
#include "LineRenderer.h"
#include "GuizmoRenderer.h"
#include "WindowsManager.h"
#include "InstanceManager.h"
#include "GuiManager.h"
#include "CloudGenerator.h"
#include "GrassRenderer.h"
#include "Vector2.h"
class RenderManager
{
public:
	static RenderManager* GetInstance() {
		static RenderManager instance;
		return &instance;
	}
	RenderManager();
	void OnWindowResize(Math::Vector2 newSize);
	void OnViewportResize(Math::Vector2 topLeft, Math::Vector2 newSize);
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

	Math::Vector2	    GetViewportSize() const     { return m_currentViewportSize; }
	Math::Vector2	    GetViewportPos() const     { return m_currentViewportPos; }

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
	CloudGenerator m_cloudGenerator;
	GrassRenderer m_grassRenderer;
    FXAA m_FXAA;
	std::unique_ptr<PerspectiveCamera> m_camera;

	Math::Vector2 m_currentViewportSize;
	Math::Vector2 m_currentViewportPos;

};
