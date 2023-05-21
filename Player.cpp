#include "Player.h"
#include <chrono>
#include <iostream>




Player::Player(Mesh& physicsMesh, SDL_Window* window, SDL_Event& event)
    : m_yaw(0.0f)
    , m_pitch(0.0f)
    , m_window(window)
    , m_event(event)
    , m_isGrounded(false)
    , m_targetVelocity(Math::Vector3(0, 0, 0))
{
	m_position.x = -0.0f;
	m_position.y = 0.0f;
    m_position.z = 0.0f;

    m_previousPosition = m_position;

	m_direction.x = 0.0f;
	m_direction.y = 0.0f;
    m_direction.z = 1.0f;	
    
    m_cameraPlane.x = 0.0f;
    m_cameraPlane.y = 0.0f;
    m_cameraPlane.z = 0.0f;

    m_physicsMesh = &physicsMesh;
    PhysXManager& physXManager = PhysXManager::GetInstance();
    //physXManager.m_player->addForce(physx::PxVec3(0.0f, 100.0f, 0.0f), physx::PxForceMode::eACCELERATION);

    auto* physics = physXManager.GetPhysics();

    //physXManager.m_player->addForce(physx::PxVec3(0.0f, 100.0f, 0.0f), physx::PxForceMode::eACCELERATION);

    physx::PxMaterial* defaultMaterial = physXManager.GetPhysics()->createMaterial(0.5f, 0.5f, 0.1f);
    PxShapeFlags shapeFlags = PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE;
    PxBoxGeometry boxGeometry(physx::PxVec3(1.0f, 1.0f, 1.0f));
    physx::PxTransform boxTransform(physx::PxVec3(m_position.x, m_position.y, m_position.z));
    PxShape* playerShape = physXManager.GetPhysics()->createShape(boxGeometry, *defaultMaterial, true, shapeFlags);
    m_physicsObject = PxCreateDynamic(*physics, boxTransform, boxGeometry, *defaultMaterial, 1.0f);
    // Attach player's physics shape to player's physics object
    m_physicsObject->attachShape(*playerShape);
    
    // Add player to scene
    physXManager.GetScene()->addActor(*m_physicsObject);







}	
Player::~Player()
{

}

float Player::calculateDeltaTime()
{
    // REPLACE TO SDL2 TICK SOMETIME
    static auto previousTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - previousTime).count();

    constexpr double targetFrameTime = 1.0f / 60.0f;
    deltaTime = std::min(deltaTime, 0.1f);
    previousTime = currentTime;

    return deltaTime;
}

void Player::Update()
{
    m_velocity = m_previousPosition - m_position;
    m_deltaTime = calculateDeltaTime();
    HandleInput();

    PhysXManager& physXManager = PhysXManager::GetInstance();
    //auto pos = m_physicsObject->getGlobalPose().p;
    //
    //m_position.x = pos.x;
    //m_position.y = pos.y;
    //m_position.z = pos.z;

    //HandlePhysics();
    HandleCamera();
}


void Player::HandleInput()
{
    float runSpeed = 2.0f;
    float walkSpeed = 15.0f;

    float currentSpeed = walkSpeed;
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    if (keystate[SDL_SCANCODE_LSHIFT]) {
        currentSpeed = walkSpeed * runSpeed;
    }
    else
    {
        currentSpeed = walkSpeed;
    }

    Math::Vector3 xdir = m_direction;
    xdir.y = 0;
    xdir = xdir.GetNormal();
    Math::Vector3 forward(xdir);
    Math::Vector3 up(0.0f, 1.0f, 0.0f);

    Math::Vector3 left(xdir.Cross(up));
    Math::Vector3 moveDirection(0, 0, 0);

    if (keystate[SDL_SCANCODE_W]) {
        moveDirection = moveDirection + forward;
        //m_targetVelocity = m_targetVelocity + forward * (currentSpeed * m_deltaTime);
    }

    if (keystate[SDL_SCANCODE_S]) {
        moveDirection = moveDirection - forward;
        //m_targetVelocity = m_targetVelocity - forward * (currentSpeed * m_deltaTime);

    }

    if (keystate[SDL_SCANCODE_D]) {
        moveDirection = moveDirection - left;
        //m_targetVelocity = m_targetVelocity - left * (currentSpeed * m_deltaTime);
    }
    if (keystate[SDL_SCANCODE_A]) {
        moveDirection = moveDirection + left;
        //m_targetVelocity = m_targetVelocity + left * (currentSpeed * m_deltaTime);
    }

    if (keystate[SDL_SCANCODE_LCTRL]) {
        moveDirection = moveDirection - up;

    }    
    
    if (keystate[SDL_SCANCODE_SPACE]) {
        moveDirection = moveDirection + up;
        if (!m_hasPressedSpace) {
            m_targetVelocity = m_targetVelocity + up * 15;
            m_hasPressedSpace = true;


        }
    }
    else
    {
        m_hasPressedSpace = false;
    }
    if (moveDirection.Length() > 0)
    {
        m_position = m_position + moveDirection.GetNormal() * (currentSpeed * m_deltaTime);
    }
}


void Player::HandlePhysics()
{
    /*
    static const float playerColliderHeigth = 2.0f;
    static const float playerColliderWidth = 0.7f;

    if (m_targetVelocity.Length() > 0)
    {
        m_position = m_position + m_targetVelocity * m_deltaTime;
        const float dampingFactor = 0.95f;

        m_targetVelocity = m_targetVelocity * dampingFactor ;

        const float minVelocityMagnitude = 0.01f;
        if (m_targetVelocity.Length() < minVelocityMagnitude)
        {
            m_targetVelocity = Math::Vector3(0,0,0);
        }
    }

    Math::Vector3 down(0.0f, -1.0f, 0.0f);

    RaycastManager::RaycastHit hit;
    if (m_raycastManager->Raycast(m_position, down, hit, m_physicsMesh))
    {
        if (hit.distance < playerColliderHeigth)
        {

            m_isGrounded = true;
            float groundAngle = abs(hit.normal.Dot(down));

            hit.point.y += playerColliderHeigth;
            m_position = hit.point;

            if(groundAngle < 0.7f)
            {
                m_targetVelocity = m_targetVelocity + hit.normal * (30 * m_deltaTime);
                m_isGrounded = false;
            }
        }
        else
        {
            m_isGrounded = false;
            m_targetVelocity = m_targetVelocity + down *  (40 * m_deltaTime);
        }

    }

    Math::Vector3 bottomSpherePosition = m_position;
    bottomSpherePosition.y -=  playerColliderWidth-2;
    auto rayDir = m_velocity.GetNormal() - m_velocity.GetNormal() - m_velocity.GetNormal();
    if (m_raycastManager->Raycast(bottomSpherePosition, rayDir, hit, m_physicsMesh))
    {
        if (hit.distance < playerColliderWidth)
        {
            auto moveAmount = hit.normal * playerColliderWidth - hit.distance;
            if (rayDir.Dot(down) < 0)
            {
                moveAmount.y = 0;
            }
            Math::Vector3 newPosition = m_position + moveAmount;
            m_position = newPosition;
        }
    }*/
}

void Player::HandleCamera()
{
    Math::Vector3 up(0.0f, 1.0f, 0.0f);
    m_cameraPlane = m_position + Math::Vector3(0, 0, 0);

    Math::Vector3 target(0.0f, 0.0f, 1.0f);
    Math::Matrix cameraMatrixRotation = cameraMatrixRotation.MakeRotationX(m_pitch) * cameraMatrixRotation.MakeRotationY(m_yaw) * cameraMatrixRotation.MakeRotationZ(0.0f);


    m_direction = cameraMatrixRotation * target;
    target = m_position + m_direction;

    m_cameraMatrix = Math::Matrix().PointAt(m_position, target, up);
    m_cameraMatrix = m_cameraMatrix.Inverse();
}


void Player::HandleMouseMovement(Math::Vector2i mouseCursor)
{
    m_yaw -= mouseCursor.x * 0.005f;
    m_pitch += mouseCursor.y * 0.005f;

    if (m_pitch > 1.57f) m_pitch = 1.57f;
    if (m_pitch < -1.57f) m_pitch = -1.57f;
}
