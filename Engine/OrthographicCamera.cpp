#include "OrthographicCamera.h"
#include "DxMathUtils.h"
#include <DirectXMath.h>

OrthographicCamera::OrthographicCamera(Math::Vector2 size, float nearClip, float farClip)
	: Camera()
	, m_size(size)
	, m_nearClip(nearClip)
	, m_farClip(farClip)
{
}

void OrthographicCamera::CreateViewAndPerspective()
{
	m_projectionMatrix = Spectral::DxMathUtils::ToSp(DirectX::XMMatrixOrthographicRH(m_size.x, m_size.y, m_nearClip, m_farClip));
	m_viewProjectionMatrix = m_matrix * m_projectionMatrix;
	m_viewProjectionMatrix.Transpose();
}

void OrthographicCamera::SetNearClip(float nearClip)
{
	m_nearClip = nearClip;
}

void OrthographicCamera::SetFarClip(float farClip)
{
	m_farClip = farClip;
}

void OrthographicCamera::SetSize(Math::Vector2 size)
{
	m_size = size;
}
