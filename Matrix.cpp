#include "Matrix.h"

Math::Matrix::Matrix(float* matrix)
{
	register float* src = matrix;
	register float* dest = &m_matrix[0][0];
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

Math::Matrix Math::Matrix::PointAt(Math::Vector3& pos, Math::Vector3& target, Math::Vector3& up)
{
	Math::Vector3 newForward = target - pos;

	newForward = newForward.GetNormal();

	Math::Vector3 a = newForward * up.Dot(newForward);
	Math::Vector3 newUp = up - a;
	newUp = newUp.GetNormal();

	Math::Vector3 newRight = newUp.Cross(newForward);

	Math::Matrix matrix;
	matrix.m_matrix[0][0] = newRight.x;	    matrix.m_matrix[0][1] = newRight.y;		matrix.m_matrix[0][2] = newRight.z;		matrix.m_matrix[0][3] = 0.0f;
	matrix.m_matrix[1][0] = newUp.x;		matrix.m_matrix[1][1] = newUp.y;		matrix.m_matrix[1][2] = newUp.z;		matrix.m_matrix[1][3] = 0.0f;
	matrix.m_matrix[2][0] = newForward.x;	matrix.m_matrix[2][1] = newForward.y;	matrix.m_matrix[2][2] = newForward.z;	matrix.m_matrix[2][3] = 0.0f;
	matrix.m_matrix[3][0] = pos.x;			matrix.m_matrix[3][1] = pos.y;			matrix.m_matrix[3][2] = pos.z;			matrix.m_matrix[3][3] = 1.0f;
	return matrix;
}

Math::Matrix Math::Matrix::MakeTranslation(Math::Vector3& pos)
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