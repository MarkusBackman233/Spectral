#pragma once
#include "pch.h"
#include <vector>
#include "Vector3.h"
#include "Matrix.h"
#include "Mesh.h"
#include "PhysXManager.h"

class Player
{
public:
	Player();
	~Player();


	void Update(float deltaTime);

	const Math::Matrix& GetCameraMatrix(){ return m_cameraMatrix; }
	const Math::Matrix& GetCameraMatrixInversed(){ return m_cameraMatrixInversed; }
	const Math::Vector3& GetPosition() { return m_position; }
	void HandleMouseMovement();
	void HandleInput(float deltaTime);
	void HandleCamera();

	float m_yaw;
	float m_pitch;
private:

	Math::Matrix m_cameraMatrixInversed;
	Math::Matrix m_cameraMatrix;

	Math::Vector3 m_position;

	physx::PxRigidDynamic* m_physicsObject;

	float m_mouseSensitivity = 3.0f;
	bool m_mouseLock = true;


};

