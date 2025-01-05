#include "Matrix.h"
#include "DirectXMath.h"
#include "EditorUtils.h"
#include "DxMathUtils.h"
#include "MathFunctions.h"
using namespace Spectral;
using namespace Math;

Matrix::Matrix()
{
	data[0][0] = 1.0f;
	data[1][1] = 1.0f;
	data[2][2] = 1.0f;
	data[3][3] = 1.0f;
}

Matrix::Matrix(float* matrix)
{
	float* src = matrix;
	float* dest = &data[0][0];
	for (int i = 0; i < 16; ++i) {
		*dest++ = *src++;
	}
}

float* Matrix::Data() const
{
	return const_cast<float*>(&data[0][0]);
}

Matrix Matrix::GetInverse() const
{
	return DxMathUtils::ToSp(DirectX::XMMatrixInverse(nullptr, DxMathUtils::ToDx(*this)));
}

Matrix Matrix::MakeIdentity()
{
	Matrix matrix;
	matrix.data[0][0] = 1.0f;
	matrix.data[1][1] = 1.0f;
	matrix.data[2][2] = 1.0f;
	matrix.data[3][3] = 1.0f;
	return matrix;
}

Matrix Matrix::MakeRotationX(float angleRad)
{
	Matrix matrix;
	matrix.data[0][0] = 1.0f;
	matrix.data[1][1] = cosf(angleRad);
	matrix.data[1][2] = sinf(angleRad);
	matrix.data[2][1] = -sinf(angleRad);
	matrix.data[2][2] = cosf(angleRad);
	matrix.data[3][3] = 1.0f;
	return matrix;
}

void Matrix::Transpose()
{
	Matrix transposedMatrix;
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			transposedMatrix.data[i][j] = data[j][i];
		}
	}
	*this = transposedMatrix;
}

Matrix Matrix::MakeRotationY(float angleRad)
{
	Matrix matrix;
	matrix.data[0][0] = cosf(angleRad);
	matrix.data[0][2] = sinf(angleRad);
	matrix.data[2][0] = -sinf(angleRad);
	matrix.data[1][1] = 1.0f;
	matrix.data[2][2] = cosf(angleRad);
	matrix.data[3][3] = 1.0f;
	return matrix;
}

Matrix Matrix::MakeRotationZ(float angleRad)
{
	Matrix matrix;
	matrix.data[0][0] = cosf(angleRad);
	matrix.data[0][1] = sinf(angleRad);
	matrix.data[1][0] = -sinf(angleRad);
	matrix.data[1][1] = cosf(angleRad);
	matrix.data[2][2] = 1.0f;
	matrix.data[3][3] = 1.0f;
	return matrix;
}

Matrix Matrix::MakeRotationXYZ(const Vector3& xyzAngleInRad)
{
	return Matrix::MakeRotationX(xyzAngleInRad.x) * Matrix::MakeRotationY(xyzAngleInRad.y) * Matrix::MakeRotationY(xyzAngleInRad.z);
}

Matrix Matrix::MakeRotationXYZAndTranslate(const Vector3& xyzAngleInRad, const Vector3& position)
{
	Matrix matrix = MakeRotationXYZ(xyzAngleInRad);
	matrix.SetPosition(position);
	return matrix;
}

Matrix Matrix::MakePerspective(float fov, float aspectRatio, float nearClip, float farClip)
{
	return DxMathUtils::ToSp(DirectX::XMMatrixPerspectiveFovRH(fov, aspectRatio, nearClip, farClip));
}

void Matrix::LookAt(const Vector3& pos, const Vector3& target, const Vector3& up)
{
	*this = DxMathUtils::ToSp(DirectX::XMMatrixLookAtLH(DxMathUtils::ToDx(pos), DxMathUtils::ToDx(target), DxMathUtils::ToDx(up)));
}

Matrix Matrix::MakeTranslation(const Vector3& pos)
{
	Matrix matrix;
	matrix.data[0][0] = 1.0f;
	matrix.data[1][1] = 1.0f;
	matrix.data[2][2] = 1.0f;
	matrix.data[3][3] = 1.0f;
	matrix.data[3][0] = pos.x;
	matrix.data[3][1] = pos.y;
	matrix.data[3][2] = pos.z;
	return matrix;
}


Matrix Matrix::MakeRotationFromQuaternion(const Vector4& quat)
{
	Matrix matrix;
	matrix.data[0][0] = 1.0f - 2.0f * (quat.y * quat.y + quat.z * quat.z);
	matrix.data[0][1] = 2.0f * (quat.x * quat.y + quat.z * quat.w);
	matrix.data[0][2] = 2.0f * (quat.x * quat.z - quat.y * quat.w);
	matrix.data[1][0] = 2.0f * (quat.x * quat.y - quat.z * quat.w);
	matrix.data[1][1] = 1.0f - 2.0f * (quat.x * quat.x + quat.z * quat.z);
	matrix.data[1][2] = 2.0f * (quat.y * quat.z + quat.x * quat.w);
	matrix.data[2][0] = 2.0f * (quat.x * quat.z + quat.y * quat.w);
	matrix.data[2][1] = 2.0f * (quat.y * quat.z - quat.x * quat.w);
	matrix.data[2][2] = 1.0f - 2.0f * (quat.x * quat.x + quat.y * quat.y);
	matrix.data[3][3] = 1.0f;
	return matrix;
}

Matrix Matrix::MakeRotationAA(const Vector3& axis, float angle)
{
	Matrix matrix;

	float s = sinf(angle);
	float c = cosf(angle);
	float x = axis.x;
	float y = axis.y;
	float z = axis.z;

	matrix.data[0][0] = x * x * (1.0f - c) + c;
	matrix.data[0][1] = x * y * (1.0f - c) - z * s;
	matrix.data[0][2] = x * z * (1.0f - c) + y * s;

	matrix.data[1][0] = x * y * (1.0f - c) + z * s;
	matrix.data[1][1] = y * y * (1.0f - c) + c;
	matrix.data[1][2] = y * z * (1.0f - c) - x * s;

	matrix.data[2][0] = x * z * (1.0f - c) - y * s;
	matrix.data[2][1] = y * z * (1.0f - c) + x * s;
	matrix.data[2][2] = z * z * (1.0f - c) + c;

	matrix.data[3][3] = 1.0f;

	return matrix;
}

Matrix Matrix::MakeScale(const Vector3& scale)
{
	Matrix mat;
	mat.data[0][0] = scale.x;  mat.data[0][1] = 0.0f;     mat.data[0][2] = 0.0f;     mat.data[0][3] = 0.0f;
	mat.data[1][0] = 0.0f;     mat.data[1][1] = scale.y;  mat.data[1][2] = 0.0f;     mat.data[1][3] = 0.0f;
	mat.data[2][0] = 0.0f;     mat.data[2][1] = 0.0f;     mat.data[2][2] = scale.z;  mat.data[2][3] = 0.0f;
	mat.data[3][0] = 0.0f;     mat.data[3][1] = 0.0f;     mat.data[3][2] = 0.0f;     mat.data[3][3] = 1.0f;
	return mat;
}

Vector3 Matrix::GetPosition() const
{
	return Vector3(data[3][0], data[3][1], data[3][2]);
}

void Matrix::SetPosition(const Vector3& pos)
{
	data[3][0] = pos.x;
	data[3][1] = pos.y;
	data[3][2] = pos.z;
	data[3][3] = 1.0f;
}

Vector3 Matrix::GetScale() const
{
	return Vector3(RightLength(), UpLength(), FrontLength());
}

Vector4 Matrix::GetQuaternion() const
{
	Vector4 q;
	float trace = data[0][0] + data[1][1] + data[2][2];

	if (trace > 0.0f) {
		float s = 0.5f / sqrtf(trace + 1.0f);
		q.w = 0.25f / s;
		q.x = (data[2][1] - data[1][2]) * s;
		q.y = (data[0][2] - data[2][0]) * s;
		q.z = (data[1][0] - data[0][1]) * s;
	}
	else {
		if (data[0][0] > data[1][1] && data[0][0] > data[2][2]) {
			float s = 2.0f * sqrtf(1.0f + data[0][0] - data[1][1] - data[2][2]);
			q.w = (data[2][1] - data[1][2]) / s;
			q.x = 0.25f * s;
			q.y = (data[0][1] + data[1][0]) / s;
			q.z = (data[0][2] + data[2][0]) / s;
		}
		else if (data[1][1] > data[2][2]) {
			float s = 2.0f * sqrtf(1.0f + data[1][1] - data[0][0] - data[2][2]);
			q.w = (data[0][2] - data[2][0]) / s;
			q.x = (data[0][1] + data[1][0]) / s;
			q.y = 0.25f * s;
			q.z = (data[1][2] + data[2][1]) / s;
		}
		else {
			float s = 2.0f * sqrtf(1.0f + data[2][2] - data[0][0] - data[1][1]);
			q.w = (data[1][0] - data[0][1]) / s;
			q.x = (data[0][2] + data[2][0]) / s;
			q.y = (data[1][2] + data[2][1]) / s;
			q.z = 0.25f * s;
		}
	}
	return q;
}

Matrix Matrix::operator*(const Matrix& A) const
{
	Matrix matrix;
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			matrix.data[r][c] = data[r][0] * A.data[0][c] + data[r][1] * A.data[1][c] + data[r][2] * A.data[2][c] + data[r][3] * A.data[3][c];
	return matrix;
}

Vector4 Matrix::operator*(const Vector4& A) const
{
	Vector4 vector;
	vector.x = A.x * data[0][0] + A.y * data[1][0] + A.z * data[2][0] + A.w * data[3][0];
	vector.y = A.x * data[0][1] + A.y * data[1][1] + A.z * data[2][1] + A.w * data[3][1];
	vector.z = A.x * data[0][2] + A.y * data[1][2] + A.z * data[2][2] + A.w * data[3][2];
	vector.w = A.x * data[0][3] + A.y * data[1][3] + A.z * data[2][3] + A.w * data[3][3];
	return vector;
}

Vector3 Matrix::operator*(const Vector3& A) const
{
	Vector3 vector;
	vector.x = A.x * data[0][0] + A.y * data[1][0] + A.z * data[2][0];
	vector.y = A.x * data[0][1] + A.y * data[1][1] + A.z * data[2][1];
	vector.z = A.x * data[0][2] + A.y * data[1][2] + A.z * data[2][2];
	return vector;
}

Vector3 Matrix::GetRight() const
{
	return Vector3(data[0][0], data[0][1], data[0][2]).GetNormal();
}

Vector3 Matrix::GetUp() const
{
	return Vector3(data[1][0], data[1][1], data[1][2]).GetNormal();
}

Vector3 Matrix::GetFront() const
{
	return Vector3(data[2][0], data[2][1], data[2][2]).GetNormal();
}


float Matrix::RightLength() const
{
	return Vector3(data[0][0], data[0][1], data[0][2]).Length();
}

float Matrix::UpLength() const
{
	return Vector3(data[1][0], data[1][1], data[1][2]).Length();
}

float Matrix::FrontLength() const
{
	return Vector3(data[2][0], data[2][1], data[2][2]).Length();
}

void Matrix::SetRight(const Vector3& direction)
{
	data[0][0] = direction.x;
	data[0][1] = direction.y;
	data[0][2] = direction.z;
	data[0][3] = 0.0f;
}

void Matrix::SetUp(const Vector3& direction)
{
	data[1][0] = direction.x;
	data[1][1] = direction.y;
	data[1][2] = direction.z;
	data[1][3] = 0.0f;
}

void Matrix::SetFront(const Vector3& direction)
{
	data[2][0] = direction.x;
	data[2][1] = direction.y;
	data[2][2] = direction.z;
	data[2][3] = 0.0f;
}

void Matrix::OrthoNormalize()
{
	auto scale = GetScale();
	scale.x = 1.0f / scale.x;
	scale.y = 1.0f / scale.y;
	scale.z = 1.0f / scale.z;
	*this = Matrix::MakeScale(scale) * *this;
}

Vector3 Matrix::GetRotationXYZInRads() const {
	Vector3 rotation;

	// Assuming the matrix is in row-major order and is orthonormal
	if (data[0][2] < 1.0f) {
		if (data[0][2] > -1.0f) {
			rotation.y = asinf(data[0][2]); // Pitch
			rotation.x = atan2f(-data[1][2], data[2][2]); // Yaw
			rotation.z = atan2f(-data[0][1], data[0][0]); // Roll
		}
		else {
			// Not a unique solution: rotation.x - rotation.z = atan2(-m[1][0], m[1][1])
			rotation.y = -static_cast<float>(Math::PI) / 2.0f; // Pitch at -90 degrees
			rotation.x = -atan2f(data[1][0], data[1][1]); // Yaw
			rotation.z = 0.0f; // Roll
		}
	}
	else {
		// Not a unique solution: rotation.x + rotation.z = atan2(-m[1][0], m[1][1])
		rotation.y = static_cast<float>(Math::PI) / 2.0f; // Pitch at 90 degrees
		rotation.x = atan2f(data[1][0], data[1][1]); // Yaw
		rotation.z = 0.0f; // Roll
	}

	return rotation;
}