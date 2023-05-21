#pragma once
#include <SDL.h>
#include <vector>
#include "Vector3.h"
#include "Matrix.h"
//#include "RaycastManager.h"
#include "Mesh.h"
#include "PhysXManager.h"
class Player
{
public:
	Player(Mesh& m_physicsMesh, SDL_Window* window, SDL_Event& event);
	~Player();


	void Update();



	float getDeltaTime() { return m_deltaTime; }

	Math::Vector3 GetPosition(){ return m_position; }
	void SetPosition(Math::Vector3& pos){ m_position = pos; }

	Math::Vector3 GetDirection(){ return m_direction; }
	Math::Vector3 GetCameraPosition(){ return m_cameraPlane; }
	const Math::Matrix& GetCameraMatrix(){ return m_cameraMatrix; }

	void HandleMouseMovement(Math::Vector2i mouseCursor);
	void HandleInput();
	void HandlePhysics();
	void HandleCamera();


private:
	float calculateDeltaTime();
	Math::Vector3 m_targetVelocity;
	Math::Vector3 m_velocity;
	Math::Vector3 m_previousPosition;

	Math::Vector3 m_position;
	Math::Vector3 m_direction;
	Math::Vector3 m_cameraPlane;
	Math::Matrix m_cameraMatrix;

	physx::PxRigidDynamic* m_physicsObject;

	Mesh* m_physicsMesh;

	//RaycastManager* m_raycastManager;

	float m_mouseSensitivity = 3.0f;

	float m_deltaTime;

	bool m_mouseLock = true;

	float m_yaw;
	float m_pitch;

	bool m_hasPressedEscape = false;
	bool m_hasPressedSpace = false;

	bool m_isGrounded;

	SDL_Event m_event;
	SDL_Window* m_window;

};

