#pragma once
#include "pch.h"
#include "Matrix.h"

class Camera
{
public:
	Camera() {};

	Math::Matrix& GetWorldMatrix();
	const Math::Matrix& GetProjectionMatrix() const;
	const Math::Matrix& GetViewMatrix() const;
	const Math::Matrix& GetViewProjectionMatrix() const;

	virtual void CreateViewAndPerspective() = 0;

protected:

	Math::Matrix m_matrix;
	Math::Matrix m_viewMatrix;
	Math::Matrix m_projectionMatrix;
	Math::Matrix m_viewProjectionMatrix;
};

