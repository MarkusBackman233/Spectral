#include "Camera.h"

Math::Matrix& Camera::GetWorldMatrix()
{
    return m_matrix; 
}

const Math::Matrix& Camera::GetProjectionMatrix() const
{
    return m_projectionMatrix;
}

const Math::Matrix& Camera::GetViewMatrix() const
{
    return m_viewMatrix;
}

const Math::Matrix& Camera::GetViewProjectionMatrix() const
{
    return m_viewProjectionMatrix;
}