#include "Vector3.h"
#include <cmath>
#include "Matrix.h"
#include "DirectXMath.h"
#include "DxMathUtils.h"
#include <PxPhysicsAPI.h>
#include "SpectralAssert.h"

using namespace Spectral;
using namespace Math;

Vector3::Vector3()
	: x(0.0f), y(0.0f), z(0.0f)
{
}

Vector3::Vector3(const DirectX::XMFLOAT3& dxVector)
	: x(dxVector.x), y(dxVector.y), z(dxVector.z)
{
}
Math::Vector3::Vector3(const Vector4& vector4)
	: x(vector4.x), y(vector4.y), z(vector4.z)
{
}
Vector3::Vector3(float _x, float _y, float _z)
	: x(_x), y(_y), z(_z)
{
}

Math::Vector3 Math::Vector3::Swizzle(const char order[3]) const
{
	Math::Vector3 swizzledVector;

	for (size_t i = 0; i < 3; i++)
	{
		switch (order[i])
		{
		case 'x':
			*(swizzledVector.Data() + i) = x;
			break;	
		case 'y':
			*(swizzledVector.Data() + i) = y;
			break;	
		case 'z':
			*(swizzledVector.Data() + i) = z;
			break;		
		case 'o':
			*(swizzledVector.Data() + i) = 0.0f;
			break;
		}
	}


	return swizzledVector;
}

float Vector3::Dot(const Vector3& other) const
{
	return x * other.x + y * other.y + other.z;
}

float Vector3::Length() const
{
	return std::sqrt(LengthSquared());
}

float Vector3::LengthSquared() const
{
	return x * x + y * y + z * z;
}

float Math::Vector3::HorizontalLength(const Vector3& other) const
{
	float dx = x - other.x;
	float dz = z - other.z;

	return sqrt(dx * dx + dz * dz);
}

float* Vector3::Data() const
{
	return const_cast<float*>(&x);
}

Vector3 Vector3::Reflect(const Vector3& normal) const
{
	float dotProduct = Dot(normal);
	Vector3 scaledNormal = normal * (2.0f * dotProduct);
	return *this - scaledNormal;
}

Vector3 Vector3::Transform(const Matrix& matrix) const
{
	DirectX::XMVECTOR dxVector = DxMathUtils::ToDx(*this);
	DirectX::XMMATRIX dxMatrix = DxMathUtils::ToDx(matrix);
	DirectX::XMVECTOR transformedVector = DirectX::XMVector3TransformCoord(dxVector, dxMatrix);
	return Vector3(transformedVector.m128_f32[0], transformedVector.m128_f32[1], transformedVector.m128_f32[2]);
}

Vector3 Vector3::TransformNormal(const Matrix& matrix) const
{
	DirectX::XMVECTOR dxVector = DxMathUtils::ToDx(*this);
	DirectX::XMMATRIX dxMatrix = DxMathUtils::ToDx(matrix);

	DirectX::XMVECTOR transformedNormal = DirectX::XMVector3TransformNormal(dxVector, dxMatrix);
	return Vector3(transformedNormal.m128_f32[0], transformedNormal.m128_f32[1], transformedNormal.m128_f32[2]);
}

Vector3 Vector3::GetInterpolate(const Vector3& A, float fraction) const
{
	Vector3 interpolatedVector;

	float invertedFraction = 1.0f - fraction;

	interpolatedVector.x = x * invertedFraction;
	interpolatedVector.y = y * invertedFraction;
	interpolatedVector.z = z * invertedFraction;

	interpolatedVector.x += A.x * fraction;
	interpolatedVector.y += A.y * fraction;
	interpolatedVector.z += A.z * fraction;

	return interpolatedVector;

}

Vector3 Vector3::GetNormal() const
{
	float length = Length();
	Vector3 vector(x / length, y / length, z / length);

	AssertAndReturn(vector.IsFinite(), "A value in Vector3 is no longer finite after normalization. Probably tried to normalize a zero vector", return Vector3());
	return vector;
}

Vector3& Math::Vector3::Normalize()
{
	float length = Length();
	x /= length;
	y /= length;
	z /= length;

	Assert(IsFinite(), "A value in Vector3 is no longer finite after normalization. Probably tried to normalize a zero vector");

	return *this;
}


Vector3 Vector3::Cross(const Vector3& in) const
{
	return Vector3(
		y * in.z - z * in.y,
		z * in.x - x * in.z,
		x * in.y - y * in.x);
}

bool Vector3::IsFinite() const
{
	return isfinite(x) && isfinite(y) && isfinite(z);
}
