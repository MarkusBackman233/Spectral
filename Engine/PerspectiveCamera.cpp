#include "PerspectiveCamera.h"
#include "MathFunctions.h"
#include <DirectXCollision.h>
#include "DxMathUtils.h"
PerspectiveCamera::PerspectiveCamera(float fovDegrees, float nearClip, float farClip, Math::Vector2 size)
    : Camera()
    , m_fov(Math::ConvertToRadians(fovDegrees))
    , m_nearClip(nearClip)
    , m_farClip(farClip)
    , m_aspectRatio(1.0f)
{
    UpdateAspectRatio(size);
}

void PerspectiveCamera::CreateViewAndPerspective()
{
    m_viewMatrix = m_matrix.GetInverse();
    auto proj = DirectX::XMMatrixPerspectiveFovLH(m_fov, m_aspectRatio, m_nearClip, m_farClip);
    m_projectionMatrix = Spectral::DxMathUtils::ToSp(proj);
    DirectX::BoundingFrustum::CreateFromMatrix(m_frustum, proj);
    m_frustum.Transform(m_frustum, Spectral::DxMathUtils::ToDx(m_matrix));


    m_viewProjectionMatrix = m_viewMatrix * m_projectionMatrix;
    m_viewProjectionMatrix.Transpose();
}

void PerspectiveCamera::UpdateAspectRatio(Math::Vector2 size)
{
    AssertAndReturn(size.y > 0.0f, "Cant update aspect ratio due to window y being 0", return);

    m_aspectRatio = size.x / size.y;
}
