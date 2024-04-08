#include "Vector3.h"
#include <cmath>


Math::Vector3::Vector3(physx::PxVec3 physxVector)
{
	x = physxVector.x;
	y = physxVector.y;
	z = physxVector.z;
	w = 0.0f;
}


Math::Vector3::Vector3(float _x, float _y, float _z)
	: x(_x), y(_y), z(_z)
{
}
Math::Vector3::Vector3(float _x, float _y, float _z, float _w)
	: x(_x), y(_y), z(_z), w(_w)
{
}

Math::Vector3 Math::Vector3::Interpolate(const Vector3& A, float fraction)
{
	Math::Vector3 interpolatedVector;

	float invertedFraction = 1.0f - fraction;

	interpolatedVector.x = x * invertedFraction;
	interpolatedVector.y = y * invertedFraction;
	interpolatedVector.z = z * invertedFraction;

	interpolatedVector.x += A.x * fraction;
	interpolatedVector.y += A.y * fraction;
	interpolatedVector.z += A.z * fraction;

	return interpolatedVector;

}

Math::Vector3 Math::Vector3::GetNormal()
{
	float length = sqrt(x * x + y * y + z * z);
	return Math::Vector3(x / length, y / length, z / length);
}

Math::Vector3 Math::Vector3::GetNegative()
{
	x = -x;
	y = -y;
	z = -z;
	return *this;
}

Math::Vector3 Math::Vector3::Cross(const Math::Vector3& in) const
{
	return Math::Vector3(
		y * in.z - z * in.y,
		z * in.x - x * in.z,
		x * in.y - y * in.x);
}

Math::Vector3 Math::Vector3::IntersectPlane(const Math::Vector3& planeNormal,const Math::Vector3& lineStart, const Math::Vector3& lineEnd, float& t) const
{
	float ad = lineStart.Dot(planeNormal);
	float bd = lineEnd.Dot(planeNormal);
	t = (planeNormal.Dot(*this) - ad) / (bd - ad);
	return lineStart + (lineEnd - lineStart) * t;
}


Math::Vector3 Math::Vector3::EulerToQuat() const
{
	static constexpr float degreesToRad = 3.1415926f / 180.0f;

	return Math::Vector3(x / degreesToRad,y / degreesToRad,z / degreesToRad);
}
