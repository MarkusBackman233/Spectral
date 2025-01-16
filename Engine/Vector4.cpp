#include "Vector4.h"
#include <cmath>
#include "Matrix.h"
#include "DirectXMath.h"
#include "DxMathUtils.h"
#include "SpectralAssert.h"

using namespace Spectral;
using namespace Math;

Vector4::Vector4()
	: x(0.0f), y(0.0f), z(0.0f), w(1.0f)
{}
Vector4::Vector4(const DirectX::XMFLOAT4& dxVector)
	: x(dxVector.x), y(dxVector.y), z(dxVector.z), w(1.0f)
{}
Vector4::Vector4(const Vector3& vector3, float _w)
	: x(vector3.x), y(vector3.y), z(vector3.z), w(_w)
{}
Vector4::Vector4(float scalar)
	: x(scalar)
	, y(scalar)
	, z(scalar)
	, w(scalar)
{}
Vector4::Vector4(float _x, float _y, float _z)
	: x(_x), y(_y), z(_z), w(1.0f)
{}
Vector4::Vector4(float _x, float _y, float _z, float _w)
	: x(_x), y(_y), z(_z), w(_w)
{}

float Vector4::Dot(const Vector4& other) const
{
	return x * other.x + y * other.y + other.z;
}

float Vector4::Length() const
{
	return sqrt(LengthSquared());
}

float Vector4::LengthSquared() const
{
	return x * x + y * y + z * z;
}

float* Vector4::Data() const
{
	return const_cast<float*>(&x);
}

Vector4 Vector4::Transform(const Matrix& matrix) const
{
	return DxMathUtils::ToSp(DirectX::XMVector3TransformCoord(DxMathUtils::ToDx(*this), DxMathUtils::ToDx(matrix)));
}

bool Vector4::IsFinite() const
{
	return isfinite(x) && isfinite(y) && isfinite(z) && isfinite(w);
}
