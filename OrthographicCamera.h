#pragma once
#include "Camera.h"
#include "Vector2.h"

class OrthographicCamera :
	public Camera
{
public:
	OrthographicCamera(Math::Vector2 size, float nearClip, float farClip);
	virtual void CreateViewAndPerspective() override;

	void SetNearClip(float nearClip);
	void SetFarClip(float farClip);
	void SetSize(Math::Vector2 size);

private:
	float m_nearClip;
	float m_farClip;
	Math::Vector2 m_size;
};

