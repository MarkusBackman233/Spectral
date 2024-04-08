#include "Matrix.h"

Math::Matrix::Matrix(float* matrix)
{
	float* src = matrix;
	float* dest = &m_matrix[0][0];
	for (int i = 0; i < 16; ++i) {
		*dest++ = *src++;
	}
}

Math::Matrix Math::Matrix::Inverse()
{
	Math::Matrix matrix;
	matrix.m_matrix[0][0] = m_matrix[0][0]; matrix.m_matrix[0][1] = m_matrix[1][0]; matrix.m_matrix[0][2] = m_matrix[2][0]; matrix.m_matrix[0][3] = 0.0f;
	matrix.m_matrix[1][0] = m_matrix[0][1]; matrix.m_matrix[1][1] = m_matrix[1][1]; matrix.m_matrix[1][2] = m_matrix[2][1]; matrix.m_matrix[1][3] = 0.0f;
	matrix.m_matrix[2][0] = m_matrix[0][2]; matrix.m_matrix[2][1] = m_matrix[1][2]; matrix.m_matrix[2][2] = m_matrix[2][2]; matrix.m_matrix[2][3] = 0.0f;
	matrix.m_matrix[3][0] = -(m_matrix[3][0] * matrix.m_matrix[0][0] + m_matrix[3][1] * matrix.m_matrix[1][0] + m_matrix[3][2] * matrix.m_matrix[2][0]);
	matrix.m_matrix[3][1] = -(m_matrix[3][0] * matrix.m_matrix[0][1] + m_matrix[3][1] * matrix.m_matrix[1][1] + m_matrix[3][2] * matrix.m_matrix[2][1]);
	matrix.m_matrix[3][2] = -(m_matrix[3][0] * matrix.m_matrix[0][2] + m_matrix[3][1] * matrix.m_matrix[1][2] + m_matrix[3][2] * matrix.m_matrix[2][2]);
	matrix.m_matrix[3][3] = 1.0f;
	return matrix;
}


Math::Matrix Math::Matrix::MakeIdentity()
{
	Math::Matrix matrix;
	matrix.m_matrix[0][0] = 1.0f;
	matrix.m_matrix[1][1] = 1.0f;
	matrix.m_matrix[2][2] = 1.0f;
	matrix.m_matrix[3][3] = 1.0f;
	return matrix;
}

Math::Matrix Math::Matrix::MakeRotationX(float angleRad)
{
	Math::Matrix matrix;
	matrix.m_matrix[0][0] = 1.0f;
	matrix.m_matrix[1][1] = cosf(angleRad);
	matrix.m_matrix[1][2] = sinf(angleRad);
	matrix.m_matrix[2][1] = -sinf(angleRad);
	matrix.m_matrix[2][2] = cosf(angleRad);
	matrix.m_matrix[3][3] = 1.0f;
	return matrix;
}

Math::Matrix Math::Matrix::MakeRotationY(float angleRad)
{
	Math::Matrix matrix;
	matrix.m_matrix[0][0] = cosf(angleRad);
	matrix.m_matrix[0][2] = sinf(angleRad);
	matrix.m_matrix[2][0] = -sinf(angleRad);
	matrix.m_matrix[1][1] = 1.0f;
	matrix.m_matrix[2][2] = cosf(angleRad);
	matrix.m_matrix[3][3] = 1.0f;
	return matrix;
}

Math::Matrix Math::Matrix::MakeRotationZ(float angleRad)
{
	Math::Matrix matrix;
	matrix.m_matrix[0][0] = cosf(angleRad);
	matrix.m_matrix[0][1] = sinf(angleRad);
	matrix.m_matrix[1][0] = -sinf(angleRad);
	matrix.m_matrix[1][1] = cosf(angleRad);
	matrix.m_matrix[2][2] = 1.0f;
	matrix.m_matrix[3][3] = 1.0f;
	return matrix;
}

void Math::Matrix::PointAt(const Math::Vector3& pos, const Math::Vector3& target, const Math::Vector3& up)
{
	Math::Vector3 newForward = target - pos;

	newForward = newForward.GetNormal();

	Math::Vector3 a = newForward * up.Dot(newForward);
	Math::Vector3 newUp = up - a;
	newUp = newUp.GetNormal();

	Math::Vector3 newRight = newUp.Cross(newForward);

	m_matrix[0][0] = newRight.x;	m_matrix[0][1] = newRight.y;	m_matrix[0][2] = newRight.z;	m_matrix[0][3] = 0.0f;
	m_matrix[1][0] = newUp.x;		m_matrix[1][1] = newUp.y;		m_matrix[1][2] = newUp.z;		m_matrix[1][3] = 0.0f;
	m_matrix[2][0] = newForward.x;	m_matrix[2][1] = newForward.y;	m_matrix[2][2] = newForward.z;	m_matrix[2][3] = 0.0f;
	m_matrix[3][0] = pos.x;			m_matrix[3][1] = pos.y;			m_matrix[3][2] = pos.z;			m_matrix[3][3] = 1.0f;

}

Math::Matrix Math::Matrix::MakeTranslation(const Math::Vector3& pos)
{
	Math::Matrix matrix;
	matrix.m_matrix[0][0] = 1.0f;
	matrix.m_matrix[1][1] = 1.0f;
	matrix.m_matrix[2][2] = 1.0f;
	matrix.m_matrix[3][3] = 1.0f;
	matrix.m_matrix[3][0] = pos.x;
	matrix.m_matrix[3][1] = pos.y;
	matrix.m_matrix[3][2] = pos.z;
	return matrix;
}


Math::Matrix Math::Matrix::MakeRotationQuaternion(const Math::Vector3& quat)
{
	Math::Matrix matrix;
	matrix.m_matrix[0][0] = 1.0f - 2.0f * (quat.y * quat.y + quat.z * quat.z);
	matrix.m_matrix[0][1] = 2.0f * (quat.x * quat.y + quat.z * quat.w);
	matrix.m_matrix[0][2] = 2.0f * (quat.x * quat.z - quat.y * quat.w);
	matrix.m_matrix[1][0] = 2.0f * (quat.x * quat.y - quat.z * quat.w);
	matrix.m_matrix[1][1] = 1.0f - 2.0f * (quat.x * quat.x + quat.z * quat.z);
	matrix.m_matrix[1][2] = 2.0f * (quat.y * quat.z + quat.x * quat.w);
	matrix.m_matrix[2][0] = 2.0f * (quat.x * quat.z + quat.y * quat.w);
	matrix.m_matrix[2][1] = 2.0f * (quat.y * quat.z - quat.x * quat.w);
	matrix.m_matrix[2][2] = 1.0f - 2.0f * (quat.x * quat.x + quat.y * quat.y);
	matrix.m_matrix[3][3] = 1.0f;
	return matrix;
}

Math::Matrix Math::Matrix::MakeRotationAA(Math::Vector3 axis, float angle)
{
	axis.GetNormal();

	Math::Matrix matrix;
	float s = sin(angle);
	float c = cos(angle);
	float x = axis.x;
	float y = axis.y;
	float z = axis.z;

	matrix.m_matrix[0][0] = x * x * (1.0f - c) + c;
	matrix.m_matrix[0][1] = x * y * (1.0f - c) - z * s;
	matrix.m_matrix[0][2] = x * z * (1.0f - c) + y * s;

	matrix.m_matrix[1][0] = x * y * (1.0f - c) + z * s;
	matrix.m_matrix[1][1] = y * y * (1.0f - c) + c;
	matrix.m_matrix[1][2] = y * z * (1.0f - c) - x * s;

	matrix.m_matrix[2][0] = x * z * (1.0f - c) - y * s;
	matrix.m_matrix[2][1] = y * z * (1.0f - c) + x * s;
	matrix.m_matrix[2][2] = z * z * (1.0f - c) + c;

	matrix.m_matrix[3][3] = 1.0f;

	return matrix;
}

Math::Matrix Math::Matrix::Scale(const Math::Vector3& scale)
{
	Math::Matrix mat;
	mat.m_matrix[0][0] = scale.x;  mat.m_matrix[0][1] = 0.0f;     mat.m_matrix[0][2] = 0.0f;     mat.m_matrix[0][3] = 0.0f;
	mat.m_matrix[1][0] = 0.0f;     mat.m_matrix[1][1] = scale.y;  mat.m_matrix[1][2] = 0.0f;     mat.m_matrix[1][3] = 0.0f;
	mat.m_matrix[2][0] = 0.0f;     mat.m_matrix[2][1] = 0.0f;     mat.m_matrix[2][2] = scale.z;  mat.m_matrix[2][3] = 0.0f;
	mat.m_matrix[3][0] = 0.0f;     mat.m_matrix[3][1] = 0.0f;     mat.m_matrix[3][2] = 0.0f;     mat.m_matrix[3][3] = 1.0f;
	return mat;
}

Math::Vector3 Math::Matrix::GetScale()
{
	return Math::Vector3(RightLength(), UpLength(), FrontLength());
}

Math::Vector3 Math::Matrix::MatrixToQuaternion() const
{
	Vector3 q;
	float trace = m_matrix[0][0] + m_matrix[1][1] + m_matrix[2][2];

	if (trace > 0.0f) {
		float s = 0.5f / sqrt(trace + 1.0f);
		q.w = 0.25f / s;
		q.x = (m_matrix[2][1] - m_matrix[1][2]) * s;
		q.y = (m_matrix[0][2] - m_matrix[2][0]) * s;
		q.z = (m_matrix[1][0] - m_matrix[0][1]) * s;
	}
	else {
		if (m_matrix[0][0] > m_matrix[1][1] && m_matrix[0][0] > m_matrix[2][2]) {
			float s = 2.0f * sqrt(1.0f + m_matrix[0][0] - m_matrix[1][1] - m_matrix[2][2]);
			q.w = (m_matrix[2][1] - m_matrix[1][2]) / s;
			q.x = 0.25f * s;
			q.y = (m_matrix[0][1] + m_matrix[1][0]) / s;
			q.z = (m_matrix[0][2] + m_matrix[2][0]) / s;
		}
		else if (m_matrix[1][1] > m_matrix[2][2]) {
			float s = 2.0f * sqrt(1.0f + m_matrix[1][1] - m_matrix[0][0] - m_matrix[2][2]);
			q.w = (m_matrix[0][2] - m_matrix[2][0]) / s;
			q.x = (m_matrix[0][1] + m_matrix[1][0]) / s;
			q.y = 0.25f * s;
			q.z = (m_matrix[1][2] + m_matrix[2][1]) / s;
		}
		else {
			float s = 2.0f * sqrt(1.0f + m_matrix[2][2] - m_matrix[0][0] - m_matrix[1][1]);
			q.w = (m_matrix[1][0] - m_matrix[0][1]) / s;
			q.x = (m_matrix[0][2] + m_matrix[2][0]) / s;
			q.y = (m_matrix[1][2] + m_matrix[2][1]) / s;
			q.z = 0.25f * s;
		}
	}
	return q;
}

Math::Vector3 Math::Matrix::Right() const
{
	return Math::Vector3(m_matrix[0][0], m_matrix[0][1], m_matrix[0][2], m_matrix[0][3]).GetNormal();
}

Math::Vector3 Math::Matrix::Up() const
{
	return Math::Vector3(m_matrix[1][0], m_matrix[1][1], m_matrix[1][2], m_matrix[1][3]).GetNormal();
}

Math::Vector3 Math::Matrix::Front() const
{
	return Math::Vector3(m_matrix[2][0], m_matrix[2][1], m_matrix[2][2], m_matrix[2][3]).GetNormal();
}


float Math::Matrix::RightLength() const
{
	return sqrt(Math::Vector3(m_matrix[0][0], m_matrix[0][1], m_matrix[0][2], m_matrix[0][3]).Length());
}

float Math::Matrix::UpLength() const
{
	return sqrt(Math::Vector3(m_matrix[1][0], m_matrix[1][1], m_matrix[1][2], m_matrix[1][3]).Length());
}

float Math::Matrix::FrontLength() const
{
	return sqrt(Math::Vector3(m_matrix[2][0], m_matrix[2][1], m_matrix[2][2], m_matrix[2][3]).Length());
}

void Math::Matrix::SetRight(const Math::Vector3& direction)
{
	m_matrix[0][0] = direction.x;
	m_matrix[0][1] = direction.y;
	m_matrix[0][2] = direction.z;
	m_matrix[0][3] = 0.0f;
}

void Math::Matrix::SetUp(const Math::Vector3& direction)
{
	m_matrix[1][0] = direction.x;
	m_matrix[1][1] = direction.y;
	m_matrix[1][2] = direction.z;
	m_matrix[1][3] = 0.0f;
}

void Math::Matrix::SetFront(const Math::Vector3& direction)
{
	m_matrix[2][0] = direction.x;
	m_matrix[2][1] = direction.y;
	m_matrix[2][2] = direction.z;
	m_matrix[2][3] = 0.0f;
}

void Math::Matrix::OrthoNormalize()
{
	// Normalize right, up, and forward vectors
	Math::Vector3 right = Right();
	Math::Vector3 up = Up();
	Math::Vector3 forward = Front();

	right.GetNormal();
	up.GetNormal();
	forward.GetNormal();

	// Make right vector orthogonal to up and forward
	right = right - up * up.Dot(right);
	right = right - forward * forward.Dot(right);
	right.GetNormal();

	// Make up vector orthogonal to right and forward
	up = up - right * right.Dot(up);
	up = up - forward * forward.Dot(up);
	up.GetNormal();

	// Make forward vector orthogonal to right and up
	forward = forward - right * right.Dot(forward);
	forward = forward - up * up.Dot(forward);
	forward.GetNormal();

	// Set the matrix with the new orthogonalized vectors
	SetRight(right);
	SetUp(up);
	SetFront(forward);
}

