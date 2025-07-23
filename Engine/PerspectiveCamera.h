#pragma once
#include "Camera.h"
#include "Vector2.h"
class PerspectiveCamera :
    public Camera
{
public:
	PerspectiveCamera(float fovDegrees, float nearClip, float farClip, Math::Vector2 size);

	virtual void CreateViewAndPerspective() override;
	void UpdateAspectRatio(Math::Vector2 size);

	float m_fov;
	float m_nearClip;
	float m_farClip;
	float m_aspectRatio;
private:

};

