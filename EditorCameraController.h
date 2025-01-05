#pragma once
#include "Matrix.h"

class EditorCameraController
{
public:
	EditorCameraController();
	void Update(float deltaTime);
	Math::Matrix& GetWorldMatrix();

private:
	void HandleRotation();
	void HandleTranslation(float deltaTime);
	void HandleCamera();

	Math::Matrix m_matrix;

	float m_yaw;
	float m_pitch;
	float m_mouseSensitivity;
};

