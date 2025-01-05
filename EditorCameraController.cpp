#include "EditorCameraController.h"
#include "Camera.h"
#include "iInput.h"
#include "iRender.h"

using namespace Math;

EditorCameraController::EditorCameraController() 
    : m_yaw(0.0f)
    , m_pitch(0.0f)
    , m_mouseSensitivity(0.004f)
{
}

void EditorCameraController::Update(float deltaTime)
{
    HandleRotation();
    HandleTranslation(deltaTime);
    HandleCamera();
}

void EditorCameraController::HandleTranslation(float deltaTime)
{
    if (!Input::GetKeyHeld(InputId::Mouse2))
    {
        return;
    }

    const float speed = Input::GetKeyHeld(InputId::Shift) ? 30.0f : 15.0f;

    const Vector3 up(0.0f, 1.0f, 0.0f);
    const Vector3 right = m_matrix.GetRight();
    const Vector3 front = up.Cross(right);

    Vector3 moveDirection(0.0f);
    if (Input::GetKeyHeld(InputId::W)) moveDirection += front;
    if (Input::GetKeyHeld(InputId::S)) moveDirection -= front;
    if (Input::GetKeyHeld(InputId::D)) moveDirection += right;
    if (Input::GetKeyHeld(InputId::A)) moveDirection -= right;
    if (Input::GetKeyHeld(InputId::Space) || Input::GetKeyHeld(InputId::Q)) moveDirection += up;
    if (Input::GetKeyHeld(InputId::Control) || Input::GetKeyHeld(InputId::Z)) moveDirection -= up;

    if (moveDirection.Length() > 0.1f)
    {
        m_matrix.SetPosition(m_matrix.GetPosition() + moveDirection.GetNormal() * speed * deltaTime);
    }
}

void EditorCameraController::HandleCamera()
{
    m_matrix = Matrix::MakeRotationXYZAndTranslate(Vector3(m_pitch, m_yaw, 0.0f), m_matrix.GetPosition());
    Render::GetCamera()->GetWorldMatrix() = m_matrix;
}

void EditorCameraController::HandleRotation()
{
    const float halfPi = 1.57f;
    const auto mouseMovement = Input::GetMouseMovement() * m_mouseSensitivity;
    if (Input::GetKeyHeld(InputId::Mouse2))
    {
        m_pitch = std::clamp(m_pitch - mouseMovement.y, -halfPi, halfPi);
        m_yaw += mouseMovement.x;
    }
}

Matrix& EditorCameraController::GetWorldMatrix()
{
    return m_matrix;
}