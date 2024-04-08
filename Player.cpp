#include "Player.h"
#include "Spectral.h"
#include "src/IMGUI/imgui_impl_dx11.h"
#include <iostream>
Player::Player()
    : m_yaw(0.0f)
    , m_pitch(0.0f)
    , m_position(0,2,-5)
{}
Player::~Player()
{

}

void Player::Update(float deltaTime)
{
    HandleInput(deltaTime);

    HandleCamera();
}


void Player::HandleInput(float deltaTime)
{
    deltaTime = Spectral::GetInstance()->GetDeltaTime();
    float runSpeed = 2.0f;
    float walkSpeed = 15.0f;

    HandleMouseMovement();
    if (!ImGui::IsKeyDown(ImGuiKey_MouseRight)) {
        return;
    }
    float currentSpeed = walkSpeed;
    if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
    {
        currentSpeed = walkSpeed * runSpeed;
    }
    else
    {
        currentSpeed = walkSpeed;
    }
    
    auto front = m_cameraMatrix.Right().Cross(Math::Vector3(0, 1, 0));
    
    Math::Vector3 moveDirection(0, 0, 0);
    
    if (ImGui::IsKeyDown(ImGuiKey_W))
    {
        moveDirection = moveDirection + front;
    }
    if (ImGui::IsKeyDown(ImGuiKey_S))
    {
        moveDirection = moveDirection - front;
    }
    if (ImGui::IsKeyDown(ImGuiKey_D))
    {
        moveDirection = moveDirection - m_cameraMatrix.Right();
    }
    if (ImGui::IsKeyDown(ImGuiKey_A))
    {
        moveDirection = moveDirection + m_cameraMatrix.Right();
    }
    
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_Z)) {
        moveDirection = moveDirection - Math::Vector3(0, 1, 0);
    
    }    
    if (ImGui::IsKeyDown(ImGuiKey_Space) || ImGui::IsKeyDown(ImGuiKey_Q)) {
        moveDirection = moveDirection + Math::Vector3(0, 1, 0);
    }

    if (moveDirection.Length() > 0)
    {
        m_position += moveDirection.GetNormal() * (currentSpeed * deltaTime);
    }
}


void Player::HandleCamera()
{
    Math::Vector3 up(0.0f, 1.0f, 0.0f);
    Math::Vector3 target(0.0f, 0.0f, 1.0f);
    Math::Matrix cameraMatrixRotation = cameraMatrixRotation.MakeRotationX(m_pitch) * cameraMatrixRotation.MakeRotationY(m_yaw) * cameraMatrixRotation.MakeRotationZ(0.0f);
    
    
    auto direction = cameraMatrixRotation * target;
    target = m_position + direction;
    m_cameraMatrix.PointAt(m_position, target, up);
    m_cameraMatrixInversed = m_cameraMatrix.Inverse();
}
void Player::HandleMouseMovement()
{

    static ImVec2 oldPos{};
    if (ImGui::IsKeyDown(ImGuiKey_MouseRight)) {
        m_pitch += (ImGui::GetMousePos().y - oldPos.y) * 0.003f;
        m_yaw += (ImGui::GetMousePos().x - oldPos.x) * 0.003f;
        if (m_pitch > 1.57f) m_pitch = 1.57f;
        if (m_pitch < -1.57f) m_pitch = -1.57f;
    }
    oldPos = ImGui::GetMousePos();
}